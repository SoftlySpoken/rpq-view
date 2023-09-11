
// Generated from ./rpq.g4 by ANTLR 4.7.2

#pragma once


#include "antlr4-runtime.h"
#include "rpqParser.h"



/**
 * This class defines an abstract visitor for a parse tree
 * produced by rpqParser.
 */
class  rpqVisitor : public antlr4::tree::AbstractParseTreeVisitor {
public:

  /**
   * Visit parse trees produced by rpqParser.
   */
    virtual antlrcpp::Any visitPath(rpqParser::PathContext *context) = 0;

    virtual antlrcpp::Any visitPathSequence(rpqParser::PathSequenceContext *context) = 0;

    virtual antlrcpp::Any visitPathElt(rpqParser::PathEltContext *context) = 0;

    virtual antlrcpp::Any visitPathMod(rpqParser::PathModContext *context) = 0;

    virtual antlrcpp::Any visitPathPrimary(rpqParser::PathPrimaryContext *context) = 0;

    virtual antlrcpp::Any visitIri(rpqParser::IriContext *context) = 0;

    virtual antlrcpp::Any visitNum_integer(rpqParser::Num_integerContext *context) = 0;


};

