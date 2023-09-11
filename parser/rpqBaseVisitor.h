
// Generated from ./rpq.g4 by ANTLR 4.7.2

#pragma once


#include "antlr4-runtime.h"
#include "rpqVisitor.h"


/**
 * This class provides an empty implementation of rpqVisitor, which can be
 * extended to create a visitor which only needs to handle a subset of the available methods.
 */
class  rpqBaseVisitor : public rpqVisitor {
public:

  virtual antlrcpp::Any visitPath(rpqParser::PathContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitPathSequence(rpqParser::PathSequenceContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitPathElt(rpqParser::PathEltContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitPathMod(rpqParser::PathModContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitPathPrimary(rpqParser::PathPrimaryContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitIri(rpqParser::IriContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitNum_integer(rpqParser::Num_integerContext *ctx) override {
    return visitChildren(ctx);
  }


};

