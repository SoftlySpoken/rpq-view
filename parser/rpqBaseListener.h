
// Generated from ./rpq.g4 by ANTLR 4.7.2

#pragma once


#include "antlr4-runtime.h"
#include "rpqListener.h"


/**
 * This class provides an empty implementation of rpqListener,
 * which can be extended to create a listener which only needs to handle a subset
 * of the available methods.
 */
class  rpqBaseListener : public rpqListener {
public:

  virtual void enterPath(rpqParser::PathContext * /*ctx*/) override { }
  virtual void exitPath(rpqParser::PathContext * /*ctx*/) override { }

  virtual void enterPathSequence(rpqParser::PathSequenceContext * /*ctx*/) override { }
  virtual void exitPathSequence(rpqParser::PathSequenceContext * /*ctx*/) override { }

  virtual void enterPathElt(rpqParser::PathEltContext * /*ctx*/) override { }
  virtual void exitPathElt(rpqParser::PathEltContext * /*ctx*/) override { }

  virtual void enterPathMod(rpqParser::PathModContext * /*ctx*/) override { }
  virtual void exitPathMod(rpqParser::PathModContext * /*ctx*/) override { }

  virtual void enterPathPrimary(rpqParser::PathPrimaryContext * /*ctx*/) override { }
  virtual void exitPathPrimary(rpqParser::PathPrimaryContext * /*ctx*/) override { }

  virtual void enterIri(rpqParser::IriContext * /*ctx*/) override { }
  virtual void exitIri(rpqParser::IriContext * /*ctx*/) override { }

  virtual void enterNum_integer(rpqParser::Num_integerContext * /*ctx*/) override { }
  virtual void exitNum_integer(rpqParser::Num_integerContext * /*ctx*/) override { }


  virtual void enterEveryRule(antlr4::ParserRuleContext * /*ctx*/) override { }
  virtual void exitEveryRule(antlr4::ParserRuleContext * /*ctx*/) override { }
  virtual void visitTerminal(antlr4::tree::TerminalNode * /*node*/) override { }
  virtual void visitErrorNode(antlr4::tree::ErrorNode * /*node*/) override { }

};

