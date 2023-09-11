
// Generated from ./rpq.g4 by ANTLR 4.7.2

#pragma once


#include "antlr4-runtime.h"
#include "rpqParser.h"


/**
 * This interface defines an abstract listener for a parse tree produced by rpqParser.
 */
class  rpqListener : public antlr4::tree::ParseTreeListener {
public:

  virtual void enterPath(rpqParser::PathContext *ctx) = 0;
  virtual void exitPath(rpqParser::PathContext *ctx) = 0;

  virtual void enterPathSequence(rpqParser::PathSequenceContext *ctx) = 0;
  virtual void exitPathSequence(rpqParser::PathSequenceContext *ctx) = 0;

  virtual void enterPathElt(rpqParser::PathEltContext *ctx) = 0;
  virtual void exitPathElt(rpqParser::PathEltContext *ctx) = 0;

  virtual void enterPathMod(rpqParser::PathModContext *ctx) = 0;
  virtual void exitPathMod(rpqParser::PathModContext *ctx) = 0;

  virtual void enterPathPrimary(rpqParser::PathPrimaryContext *ctx) = 0;
  virtual void exitPathPrimary(rpqParser::PathPrimaryContext *ctx) = 0;

  virtual void enterIri(rpqParser::IriContext *ctx) = 0;
  virtual void exitIri(rpqParser::IriContext *ctx) = 0;

  virtual void enterNum_integer(rpqParser::Num_integerContext *ctx) = 0;
  virtual void exitNum_integer(rpqParser::Num_integerContext *ctx) = 0;


};

