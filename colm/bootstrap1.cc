/*
 *  Copyright 2006-2012 Adrian Thurston <thurston@complang.org>
 */

/*  This file is part of Colm.
 *
 *  Colm is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 * 
 *  Colm is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 * 
 *  You should have received a copy of the GNU General Public License
 *  along with Colm; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 */

#include <iostream>
#include <string>
#include <errno.h>

#include "parser.h"
#include "config.h"
#include "lmparse.h"
#include "global.h"
#include "input.h"
#include "bootstrap1.h"
#include "exports1.h"
#include "colm/colm.h"

using std::string;

extern RuntimeData main_runtimeData;

void Bootstrap1::walkProdElList( ProdElList *list, prod_el_list &prodElList )
{
	if ( prodElList.ProdElList() != 0 ) {
		prod_el_list RightProdElList = prodElList.ProdElList();
		walkProdElList( list, RightProdElList );
	}
	
	if ( prodElList.ProdEl() != 0 ) {
		prod_el El = prodElList.ProdEl();
		String typeName = El.Id().text().c_str();

		ObjectField *captureField = 0;
		if ( El.OptName().Name() != 0 ) {
			String fieldName = El.OptName().Name().text().c_str();
			captureField = ObjectField::cons( internal, 0, fieldName );
		}

		RepeatType repeatType = RepeatNone;
		if ( El.OptRepeat().Star() != 0 )
			repeatType = RepeatRepeat;

		ProdEl *prodEl = prodElName( internal, typeName,
				NamespaceQual::cons(namespaceStack.top()),
				captureField, repeatType, false );

		appendProdEl( list, prodEl );
	}
}

void Bootstrap1::walkProdList( LelDefList *outProdList, prod_list &prodList )
{
	if ( prodList.ProdList() != 0 ) {
		prod_list RightProdList = prodList.ProdList();
		walkProdList( outProdList, RightProdList );
	}

	ProdElList *outElList = new ProdElList;
	prod_el_list prodElList = prodList.Prod().ProdElList();
	walkProdElList( outElList, prodElList );

	Production *prod = BaseParser::production( internal, outElList, false, 0, 0 );
	prodAppend( outProdList, prod );
}

LexFactor *Bootstrap1::walkLexFactor( lex_factor &lexFactor )
{
	if ( lexFactor.Literal() != 0 ) {
		String litString = lexFactor.Literal().text().c_str();
		Literal *literal = Literal::cons( internal, litString, Literal::LitString );
		LexFactor *factor = LexFactor::cons( literal );
		return factor;
	}
	else if ( lexFactor.Expr() != 0 ) {
		lex_expr LexExpr = lexFactor.Expr();
		LexExpression *expr = walkLexExpr( LexExpr );
		LexJoin *join = LexJoin::cons( expr );
		LexFactor *factor = LexFactor::cons( join );
		return factor;
	}
	else {
		String low = lexFactor.Low().text().c_str();
		Literal *lowLit = Literal::cons( internal, low, Literal::LitString );

		String high = lexFactor.High().text().c_str();
		Literal *highLit = Literal::cons( internal, high, Literal::LitString );

		Range *range = Range::cons( lowLit, highLit );
		LexFactor *factor = LexFactor::cons( range );
		return factor;
	}
}

LexFactorNeg *Bootstrap1::walkLexFactorNeg( lex_factor_neg &lexFactorNeg )
{
	if ( lexFactorNeg.FactorNeg() != 0 ) {
		lex_factor_neg Rec = lexFactorNeg.FactorNeg();
		LexFactorNeg *recNeg = walkLexFactorNeg( Rec );
		LexFactorNeg *factorNeg = LexFactorNeg::cons( internal, recNeg, LexFactorNeg::CharNegateType );
		return factorNeg;
	}
	else {
		lex_factor LexFactorTree = lexFactorNeg.Factor();
		LexFactor *factor = walkLexFactor( LexFactorTree );
		LexFactorNeg *factorNeg = LexFactorNeg::cons( internal, factor );
		return factorNeg;
	}
}

LexFactorRep *Bootstrap1::walkLexFactorRep( lex_factor_rep &lexFactorRep )
{
	if ( lexFactorRep.FactorRep() != 0 ) {
		lex_factor_rep Rec = lexFactorRep.FactorRep();
		LexFactorRep *recRep = walkLexFactorRep( Rec );
		LexFactorRep *factorRep = LexFactorRep::cons( internal, recRep, 0, 0, LexFactorRep::StarType );
		return factorRep;
	}
	else {
		lex_factor_neg LexFactorNegTree = lexFactorRep.FactorNeg();
		LexFactorNeg *factorNeg = walkLexFactorNeg( LexFactorNegTree );
		LexFactorRep *factorRep = LexFactorRep::cons( internal, factorNeg );
		return factorRep;
	}
}

LexFactorAug *Bootstrap1::walkLexFactorAug( lex_factor_rep &lexFactorRep )
{
	LexFactorRep *factorRep = walkLexFactorRep( lexFactorRep );
	return LexFactorAug::cons( factorRep );
}

LexTerm *Bootstrap1::walkLexTerm( lex_term &lexTerm )
{
	if ( lexTerm.Term() != 0 ) {
		lex_term Rec = lexTerm.Term();
		LexTerm *leftTerm = walkLexTerm( Rec );

		lex_factor_rep LexFactorRepTree = lexTerm.FactorRep();
		LexFactorAug *factorAug = walkLexFactorAug( LexFactorRepTree );
		LexTerm *term = LexTerm::cons( leftTerm, factorAug, LexTerm::ConcatType );
		return term;
	}
	else {
		lex_factor_rep LexFactorRepTree = lexTerm.FactorRep();
		LexFactorAug *factorAug = walkLexFactorAug( LexFactorRepTree );
		LexTerm *term = LexTerm::cons( factorAug );
		return term;
	}
}

LexExpression *Bootstrap1::walkLexExpr( lex_expr &LexExprTree )
{
	if ( LexExprTree.Expr() != 0 ) {
		lex_expr Rec = LexExprTree.Expr();
		LexExpression *leftExpr = walkLexExpr( Rec );

		lex_term lexTerm = LexExprTree.Term();
		LexTerm *term = walkLexTerm( lexTerm );
		LexExpression *expr = LexExpression::cons( leftExpr, term, LexExpression::OrType );

		return expr;
	}
	else {
		lex_term lexTerm = LexExprTree.Term();
		LexTerm *term = walkLexTerm( lexTerm );
		LexExpression *expr = LexExpression::cons( term );
		return expr;
	}
}

void Bootstrap1::walkTokenList( token_list &tokenList )
{
	if ( tokenList.TokenList() != 0 ) {
		token_list RightTokenList = tokenList.TokenList();
		walkTokenList( RightTokenList );
	}
	
	if ( tokenList.TokenDef() != 0 ) {
		token_def tokenDef = tokenList.TokenDef();
		String name = tokenDef.Id().text().c_str();

		ObjectDef *objectDef = ObjectDef::cons( ObjectDef::UserType, name, pd->nextObjectId++ ); 

		lex_expr LexExpr = tokenDef.Expr();
		LexExpression *expr = walkLexExpr( LexExpr );
		LexJoin *join = LexJoin::cons( expr );

		defineToken( internal, name, join, objectDef, 0, false, false, false );
	}

	if ( tokenList.IgnoreDef() != 0 ) {
		ignore_def IgnoreDef = tokenList.IgnoreDef();

		ObjectDef *objectDef = ObjectDef::cons( ObjectDef::UserType, 0, pd->nextObjectId++ ); 

		lex_expr LexExpr = IgnoreDef.Expr();
		LexExpression *expr = walkLexExpr( LexExpr );
		LexJoin *join = LexJoin::cons( expr );

		defineToken( internal, 0, join, objectDef, 0, true, false, false );
	}
}

void Bootstrap1::walkLexRegion( item &LexRegion )
{
	pushRegionSet( internal );

	token_list tokenList = LexRegion.TokenList();
	walkTokenList( tokenList );

	popRegionSet();
}

void Bootstrap1::walkDefinition( item &define )
{
	prod_list ProdList = define.ProdList();

	LelDefList *defList = new LelDefList;
	walkProdList( defList, ProdList );

	String name = define.DefId().text().c_str();
	NtDef *ntDef = NtDef::cons( name, namespaceStack.top(), contextStack.top(), false );
	ObjectDef *objectDef = ObjectDef::cons( ObjectDef::UserType, name, pd->nextObjectId++ ); 
	cflDef( ntDef, objectDef, defList );
}

void Bootstrap1::consParseStmt( StmtList *stmtList )
{
	NamespaceQual *nspaceQual = NamespaceQual::cons( namespaceStack.top() );
	TypeRef *typeRef = TypeRef::cons( internal, nspaceQual, String("start"), RepeatNone );

	LangVarRef *varRef = LangVarRef::cons( internal, new QualItemVect, String("stdin") );
	LangExpr *expr = LangExpr::cons( LangTerm::cons( internal, LangTerm::VarRefType, varRef ) );

	ConsItem *consItem = ConsItem::cons( internal, ConsItem::ExprType, expr );
	ConsItemList *list = ConsItemList::cons( consItem );

	ObjectField *objField = ObjectField::cons( internal, 0, String("P") );

	expr = parseCmd( internal, false, objField, typeRef, 0, list );
	LangStmt *stmt = LangStmt::cons( internal, LangStmt::ExprType, expr );
	stmtList->append( stmt );
}

void Bootstrap1::consExportStmt( StmtList *stmtList )
{
	QualItemVect *qual = new QualItemVect;
	qual->append( QualItem( internal, String( "P" ), QualItem::Dot ) );
	LangVarRef *varRef = LangVarRef::cons( internal, qual, String("tree") );
	LangExpr *expr = LangExpr::cons( LangTerm::cons( internal, LangTerm::VarRefType, varRef ) );

	NamespaceQual *nspaceQual = NamespaceQual::cons( namespaceStack.top() );
	TypeRef *typeRef = TypeRef::cons( internal, nspaceQual, String("start"), RepeatNone );
	ObjectField *program = ObjectField::cons( internal, typeRef, String("ColmTree") );
	LangStmt *programExport = exportStmt( program, LangStmt::AssignType, expr );
	stmtList->append( programExport );
}

void Bootstrap1::go()
{
	StmtList *stmtList = new StmtList;

	colmInit( 0 );
	ColmProgram *program = colmNewProgram( &main_runtimeData );
	colmRunProgram( program, 0, 0 );

	/* Extract the parse tree. */
	start Start = ColmTree( program );

	if ( Start == 0 ) {
		std::cerr << "error parsing input" << std::endl;
		return;
	}

	/* Walk the list of items. */
	_repeat_item ItemList = Start.ItemList();
	while ( !ItemList.end() ) {

		item Item = ItemList.value();
		if ( Item.DefId() != 0 )
			walkDefinition( Item );
		else if ( Item.TokenList() != 0 )
			walkLexRegion( Item );
		ItemList = ItemList.next();
	}

	colmDeleteProgram( program );

	consParseStmt( stmtList );
	consExportStmt( stmtList );

	pd->rootCodeBlock = CodeBlock::cons( stmtList, 0 );
}

