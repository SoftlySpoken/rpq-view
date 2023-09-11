
// Generated from ./rpq.g4 by ANTLR 4.7.2


#include "rpqListener.h"
#include "rpqVisitor.h"

#include "rpqParser.h"


using namespace antlrcpp;
using namespace antlr4;

rpqParser::rpqParser(TokenStream *input) : Parser(input) {
  _interpreter = new atn::ParserATNSimulator(this, _atn, _decisionToDFA, _sharedContextCache);
}

rpqParser::~rpqParser() {
  delete _interpreter;
}

std::string rpqParser::getGrammarFileName() const {
  return "rpq.g4";
}

const std::vector<std::string>& rpqParser::getRuleNames() const {
  return _ruleNames;
}

dfa::Vocabulary& rpqParser::getVocabulary() const {
  return _vocabulary;
}


//----------------- PathContext ------------------------------------------------------------------

rpqParser::PathContext::PathContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<rpqParser::PathSequenceContext *> rpqParser::PathContext::pathSequence() {
  return getRuleContexts<rpqParser::PathSequenceContext>();
}

rpqParser::PathSequenceContext* rpqParser::PathContext::pathSequence(size_t i) {
  return getRuleContext<rpqParser::PathSequenceContext>(i);
}


size_t rpqParser::PathContext::getRuleIndex() const {
  return rpqParser::RulePath;
}

void rpqParser::PathContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<rpqListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterPath(this);
}

void rpqParser::PathContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<rpqListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitPath(this);
}


antlrcpp::Any rpqParser::PathContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<rpqVisitor*>(visitor))
    return parserVisitor->visitPath(this);
  else
    return visitor->visitChildren(this);
}

rpqParser::PathContext* rpqParser::path() {
  PathContext *_localctx = _tracker.createInstance<PathContext>(_ctx, getState());
  enterRule(_localctx, 0, rpqParser::RulePath);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(14);
    pathSequence();
    setState(19);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == rpqParser::T__0) {
      setState(15);
      match(rpqParser::T__0);
      setState(16);
      pathSequence();
      setState(21);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- PathSequenceContext ------------------------------------------------------------------

rpqParser::PathSequenceContext::PathSequenceContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<rpqParser::PathEltContext *> rpqParser::PathSequenceContext::pathElt() {
  return getRuleContexts<rpqParser::PathEltContext>();
}

rpqParser::PathEltContext* rpqParser::PathSequenceContext::pathElt(size_t i) {
  return getRuleContext<rpqParser::PathEltContext>(i);
}


size_t rpqParser::PathSequenceContext::getRuleIndex() const {
  return rpqParser::RulePathSequence;
}

void rpqParser::PathSequenceContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<rpqListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterPathSequence(this);
}

void rpqParser::PathSequenceContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<rpqListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitPathSequence(this);
}


antlrcpp::Any rpqParser::PathSequenceContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<rpqVisitor*>(visitor))
    return parserVisitor->visitPathSequence(this);
  else
    return visitor->visitChildren(this);
}

rpqParser::PathSequenceContext* rpqParser::pathSequence() {
  PathSequenceContext *_localctx = _tracker.createInstance<PathSequenceContext>(_ctx, getState());
  enterRule(_localctx, 2, rpqParser::RulePathSequence);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(22);
    pathElt();
    setState(27);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == rpqParser::T__1) {
      setState(23);
      match(rpqParser::T__1);
      setState(24);
      pathElt();
      setState(29);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- PathEltContext ------------------------------------------------------------------

rpqParser::PathEltContext::PathEltContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

rpqParser::PathPrimaryContext* rpqParser::PathEltContext::pathPrimary() {
  return getRuleContext<rpqParser::PathPrimaryContext>(0);
}

rpqParser::PathModContext* rpqParser::PathEltContext::pathMod() {
  return getRuleContext<rpqParser::PathModContext>(0);
}


size_t rpqParser::PathEltContext::getRuleIndex() const {
  return rpqParser::RulePathElt;
}

void rpqParser::PathEltContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<rpqListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterPathElt(this);
}

void rpqParser::PathEltContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<rpqListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitPathElt(this);
}


antlrcpp::Any rpqParser::PathEltContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<rpqVisitor*>(visitor))
    return parserVisitor->visitPathElt(this);
  else
    return visitor->visitChildren(this);
}

rpqParser::PathEltContext* rpqParser::pathElt() {
  PathEltContext *_localctx = _tracker.createInstance<PathEltContext>(_ctx, getState());
  enterRule(_localctx, 4, rpqParser::RulePathElt);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(30);
    pathPrimary();
    setState(32);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & ((1ULL << rpqParser::T__2)
      | (1ULL << rpqParser::T__3)
      | (1ULL << rpqParser::T__4))) != 0)) {
      setState(31);
      pathMod();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- PathModContext ------------------------------------------------------------------

rpqParser::PathModContext::PathModContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t rpqParser::PathModContext::getRuleIndex() const {
  return rpqParser::RulePathMod;
}

void rpqParser::PathModContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<rpqListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterPathMod(this);
}

void rpqParser::PathModContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<rpqListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitPathMod(this);
}


antlrcpp::Any rpqParser::PathModContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<rpqVisitor*>(visitor))
    return parserVisitor->visitPathMod(this);
  else
    return visitor->visitChildren(this);
}

rpqParser::PathModContext* rpqParser::pathMod() {
  PathModContext *_localctx = _tracker.createInstance<PathModContext>(_ctx, getState());
  enterRule(_localctx, 6, rpqParser::RulePathMod);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(34);
    _la = _input->LA(1);
    if (!((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & ((1ULL << rpqParser::T__2)
      | (1ULL << rpqParser::T__3)
      | (1ULL << rpqParser::T__4))) != 0))) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- PathPrimaryContext ------------------------------------------------------------------

rpqParser::PathPrimaryContext::PathPrimaryContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

rpqParser::IriContext* rpqParser::PathPrimaryContext::iri() {
  return getRuleContext<rpqParser::IriContext>(0);
}

rpqParser::PathContext* rpqParser::PathPrimaryContext::path() {
  return getRuleContext<rpqParser::PathContext>(0);
}


size_t rpqParser::PathPrimaryContext::getRuleIndex() const {
  return rpqParser::RulePathPrimary;
}

void rpqParser::PathPrimaryContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<rpqListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterPathPrimary(this);
}

void rpqParser::PathPrimaryContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<rpqListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitPathPrimary(this);
}


antlrcpp::Any rpqParser::PathPrimaryContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<rpqVisitor*>(visitor))
    return parserVisitor->visitPathPrimary(this);
  else
    return visitor->visitChildren(this);
}

rpqParser::PathPrimaryContext* rpqParser::pathPrimary() {
  PathPrimaryContext *_localctx = _tracker.createInstance<PathPrimaryContext>(_ctx, getState());
  enterRule(_localctx, 8, rpqParser::RulePathPrimary);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    setState(41);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case rpqParser::T__7: {
        enterOuterAlt(_localctx, 1);
        setState(36);
        iri();
        break;
      }

      case rpqParser::T__5: {
        enterOuterAlt(_localctx, 2);
        setState(37);
        match(rpqParser::T__5);
        setState(38);
        path();
        setState(39);
        match(rpqParser::T__6);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- IriContext ------------------------------------------------------------------

rpqParser::IriContext::IriContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

rpqParser::Num_integerContext* rpqParser::IriContext::num_integer() {
  return getRuleContext<rpqParser::Num_integerContext>(0);
}


size_t rpqParser::IriContext::getRuleIndex() const {
  return rpqParser::RuleIri;
}

void rpqParser::IriContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<rpqListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterIri(this);
}

void rpqParser::IriContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<rpqListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitIri(this);
}


antlrcpp::Any rpqParser::IriContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<rpqVisitor*>(visitor))
    return parserVisitor->visitIri(this);
  else
    return visitor->visitChildren(this);
}

rpqParser::IriContext* rpqParser::iri() {
  IriContext *_localctx = _tracker.createInstance<IriContext>(_ctx, getState());
  enterRule(_localctx, 10, rpqParser::RuleIri);
  size_t _la = 0;

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(43);
    match(rpqParser::T__7);
    setState(44);
    num_integer();
    setState(46);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == rpqParser::T__8) {
      setState(45);
      match(rpqParser::T__8);
    }
    setState(48);
    match(rpqParser::T__9);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Num_integerContext ------------------------------------------------------------------

rpqParser::Num_integerContext::Num_integerContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* rpqParser::Num_integerContext::INTEGER() {
  return getToken(rpqParser::INTEGER, 0);
}


size_t rpqParser::Num_integerContext::getRuleIndex() const {
  return rpqParser::RuleNum_integer;
}

void rpqParser::Num_integerContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<rpqListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterNum_integer(this);
}

void rpqParser::Num_integerContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<rpqListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitNum_integer(this);
}


antlrcpp::Any rpqParser::Num_integerContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<rpqVisitor*>(visitor))
    return parserVisitor->visitNum_integer(this);
  else
    return visitor->visitChildren(this);
}

rpqParser::Num_integerContext* rpqParser::num_integer() {
  Num_integerContext *_localctx = _tracker.createInstance<Num_integerContext>(_ctx, getState());
  enterRule(_localctx, 12, rpqParser::RuleNum_integer);

  auto onExit = finally([=] {
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(50);
    match(rpqParser::INTEGER);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

// Static vars and initialization.
std::vector<dfa::DFA> rpqParser::_decisionToDFA;
atn::PredictionContextCache rpqParser::_sharedContextCache;

// We own the ATN which in turn owns the ATN states.
atn::ATN rpqParser::_atn;
std::vector<uint16_t> rpqParser::_serializedATN;

std::vector<std::string> rpqParser::_ruleNames = {
  "path", "pathSequence", "pathElt", "pathMod", "pathPrimary", "iri", "num_integer"
};

std::vector<std::string> rpqParser::_literalNames = {
  "", "'|'", "'/'", "'?'", "'*'", "'+'", "'('", "')'", "'<'", "'-'", "'>'"
};

std::vector<std::string> rpqParser::_symbolicNames = {
  "", "", "", "", "", "", "", "", "", "", "", "INTEGER"
};

dfa::Vocabulary rpqParser::_vocabulary(_literalNames, _symbolicNames);

std::vector<std::string> rpqParser::_tokenNames;

rpqParser::Initializer::Initializer() {
	for (size_t i = 0; i < _symbolicNames.size(); ++i) {
		std::string name = _vocabulary.getLiteralName(i);
		if (name.empty()) {
			name = _vocabulary.getSymbolicName(i);
		}

		if (name.empty()) {
			_tokenNames.push_back("<INVALID>");
		} else {
      _tokenNames.push_back(name);
    }
	}

  _serializedATN = {
    0x3, 0x608b, 0xa72a, 0x8133, 0xb9ed, 0x417c, 0x3be7, 0x7786, 0x5964, 
    0x3, 0xd, 0x37, 0x4, 0x2, 0x9, 0x2, 0x4, 0x3, 0x9, 0x3, 0x4, 0x4, 0x9, 
    0x4, 0x4, 0x5, 0x9, 0x5, 0x4, 0x6, 0x9, 0x6, 0x4, 0x7, 0x9, 0x7, 0x4, 
    0x8, 0x9, 0x8, 0x3, 0x2, 0x3, 0x2, 0x3, 0x2, 0x7, 0x2, 0x14, 0xa, 0x2, 
    0xc, 0x2, 0xe, 0x2, 0x17, 0xb, 0x2, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x7, 
    0x3, 0x1c, 0xa, 0x3, 0xc, 0x3, 0xe, 0x3, 0x1f, 0xb, 0x3, 0x3, 0x4, 0x3, 
    0x4, 0x5, 0x4, 0x23, 0xa, 0x4, 0x3, 0x5, 0x3, 0x5, 0x3, 0x6, 0x3, 0x6, 
    0x3, 0x6, 0x3, 0x6, 0x3, 0x6, 0x5, 0x6, 0x2c, 0xa, 0x6, 0x3, 0x7, 0x3, 
    0x7, 0x3, 0x7, 0x5, 0x7, 0x31, 0xa, 0x7, 0x3, 0x7, 0x3, 0x7, 0x3, 0x8, 
    0x3, 0x8, 0x3, 0x8, 0x2, 0x2, 0x9, 0x2, 0x4, 0x6, 0x8, 0xa, 0xc, 0xe, 
    0x2, 0x3, 0x3, 0x2, 0x5, 0x7, 0x2, 0x34, 0x2, 0x10, 0x3, 0x2, 0x2, 0x2, 
    0x4, 0x18, 0x3, 0x2, 0x2, 0x2, 0x6, 0x20, 0x3, 0x2, 0x2, 0x2, 0x8, 0x24, 
    0x3, 0x2, 0x2, 0x2, 0xa, 0x2b, 0x3, 0x2, 0x2, 0x2, 0xc, 0x2d, 0x3, 0x2, 
    0x2, 0x2, 0xe, 0x34, 0x3, 0x2, 0x2, 0x2, 0x10, 0x15, 0x5, 0x4, 0x3, 
    0x2, 0x11, 0x12, 0x7, 0x3, 0x2, 0x2, 0x12, 0x14, 0x5, 0x4, 0x3, 0x2, 
    0x13, 0x11, 0x3, 0x2, 0x2, 0x2, 0x14, 0x17, 0x3, 0x2, 0x2, 0x2, 0x15, 
    0x13, 0x3, 0x2, 0x2, 0x2, 0x15, 0x16, 0x3, 0x2, 0x2, 0x2, 0x16, 0x3, 
    0x3, 0x2, 0x2, 0x2, 0x17, 0x15, 0x3, 0x2, 0x2, 0x2, 0x18, 0x1d, 0x5, 
    0x6, 0x4, 0x2, 0x19, 0x1a, 0x7, 0x4, 0x2, 0x2, 0x1a, 0x1c, 0x5, 0x6, 
    0x4, 0x2, 0x1b, 0x19, 0x3, 0x2, 0x2, 0x2, 0x1c, 0x1f, 0x3, 0x2, 0x2, 
    0x2, 0x1d, 0x1b, 0x3, 0x2, 0x2, 0x2, 0x1d, 0x1e, 0x3, 0x2, 0x2, 0x2, 
    0x1e, 0x5, 0x3, 0x2, 0x2, 0x2, 0x1f, 0x1d, 0x3, 0x2, 0x2, 0x2, 0x20, 
    0x22, 0x5, 0xa, 0x6, 0x2, 0x21, 0x23, 0x5, 0x8, 0x5, 0x2, 0x22, 0x21, 
    0x3, 0x2, 0x2, 0x2, 0x22, 0x23, 0x3, 0x2, 0x2, 0x2, 0x23, 0x7, 0x3, 
    0x2, 0x2, 0x2, 0x24, 0x25, 0x9, 0x2, 0x2, 0x2, 0x25, 0x9, 0x3, 0x2, 
    0x2, 0x2, 0x26, 0x2c, 0x5, 0xc, 0x7, 0x2, 0x27, 0x28, 0x7, 0x8, 0x2, 
    0x2, 0x28, 0x29, 0x5, 0x2, 0x2, 0x2, 0x29, 0x2a, 0x7, 0x9, 0x2, 0x2, 
    0x2a, 0x2c, 0x3, 0x2, 0x2, 0x2, 0x2b, 0x26, 0x3, 0x2, 0x2, 0x2, 0x2b, 
    0x27, 0x3, 0x2, 0x2, 0x2, 0x2c, 0xb, 0x3, 0x2, 0x2, 0x2, 0x2d, 0x2e, 
    0x7, 0xa, 0x2, 0x2, 0x2e, 0x30, 0x5, 0xe, 0x8, 0x2, 0x2f, 0x31, 0x7, 
    0xb, 0x2, 0x2, 0x30, 0x2f, 0x3, 0x2, 0x2, 0x2, 0x30, 0x31, 0x3, 0x2, 
    0x2, 0x2, 0x31, 0x32, 0x3, 0x2, 0x2, 0x2, 0x32, 0x33, 0x7, 0xc, 0x2, 
    0x2, 0x33, 0xd, 0x3, 0x2, 0x2, 0x2, 0x34, 0x35, 0x7, 0xd, 0x2, 0x2, 
    0x35, 0xf, 0x3, 0x2, 0x2, 0x2, 0x7, 0x15, 0x1d, 0x22, 0x2b, 0x30, 
  };

  atn::ATNDeserializer deserializer;
  _atn = deserializer.deserialize(_serializedATN);

  size_t count = _atn.getNumberOfDecisions();
  _decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    _decisionToDFA.emplace_back(_atn.getDecisionState(i), i);
  }
}

rpqParser::Initializer rpqParser::_init;
