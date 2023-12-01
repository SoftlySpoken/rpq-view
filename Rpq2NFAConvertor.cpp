/**
 * @file Rpq2NFAConvertor.cpp
 * @author Yue Pang 
 * @brief Implements methods in Rpq2NFAConvertor.h
 * @date 2022-08-24
 */

#include "Rpq2NFAConvertor.h"

using namespace std;

void Rpq2NFAConvertor::getClauses(const std::string &query, std::vector<std::string> &v) {
    istringstream ifs(query);
    RpqErrorListener lstnr;

    antlr4::ANTLRInputStream input(ifs);
    rpqLexer lexer(&input);
    lexer.removeErrorListeners();
    lexer.addErrorListener(&lstnr);

    antlr4::CommonTokenStream tokens(&lexer);
    rpqParser parser(&tokens);
    parser.removeErrorListeners();
    parser.addErrorListener(&lstnr);

    rpqParser::PathContext *path = parser.path();
    if (path->pathSequence().size() > 1)
        return;
    v.clear();
    getClausesSinglePath(path, v);
}

void Rpq2NFAConvertor::getClausesSinglePath(rpqParser::PathContext *path, std::vector<std::string> &v) {
    std::vector<rpqParser::PathEltContext *> cVec = path->pathSequence()[0]->pathElt();
    for (const auto &c : cVec) {
        if (!c->pathMod() && c->pathPrimary()->path())
            getClausesSinglePath(c->pathPrimary()->path(), v);
        else
            v.emplace_back(c->getText());
    }
}

/**
 * @brief Overall driver function, returns pointer to the NFA corresponding to the query.
 * 
 * @param query the query string
 * @return shared_ptr<NFA> pointer to the NFA corresponding to the query
 */
shared_ptr<NFA> Rpq2NFAConvertor::convert(const string &query)
{
    shared_ptr<NFA> nfa_ptr = make_shared<NFA>();
    try
    {
        istringstream ifs(query);
        RpqErrorListener lstnr;

        antlr4::ANTLRInputStream input(ifs);
	    rpqLexer lexer(&input);
	    lexer.removeErrorListeners();
	    lexer.addErrorListener(&lstnr);

	    antlr4::CommonTokenStream tokens(&lexer);
	    rpqParser parser(&tokens);
	    parser.removeErrorListeners();
	    parser.addErrorListener(&lstnr);

	    rpqParser::PathContext *path = parser.path();
	    visitPath(path, nfa_ptr);
	}
    catch(const runtime_error& e1)
    {
		throw runtime_error(e1.what());
    }
    return nfa_ptr;
}

/**
 * @brief path : pathAlternative ;
 * 
 * @param ctx pointer to path's context
 * @param nfa_ptr pointer to the NFA to be constructed
 * @return antlrcpp::Any a dummy antlrcpp::Any object
 */
antlrcpp::Any
Rpq2NFAConvertor::visitPath(rpqParser::PathContext *ctx, shared_ptr<NFA> nfa_ptr)
{
    if ((ctx->pathSequence()).size() == 1)
        visitPathSequence(ctx->pathSequence(0), nfa_ptr);
    else
    {
        shared_ptr<NFA> currNfa_ptr;
        nfa_ptr->unsetAccept();
        for (auto pathSequence : ctx->pathSequence())
        {
            currNfa_ptr = make_shared<NFA>();
            visitPathSequence(pathSequence, currNfa_ptr);
            // Attach transition from current initial to sub-initial (label eps)
            nfa_ptr->initial->addTransition(-1, true, currNfa_ptr->initial);
            for (auto acceptState : currNfa_ptr->accepts)
                nfa_ptr->setAccept(acceptState);
            nfa_ptr->addStates(currNfa_ptr->states);
        }
    }
    return antlrcpp::Any();
}

/**
 * @brief pathSequence : pathEltOrInverse ( '/' pathEltOrInverse )* 
 * 
 * @param ctx pointer to pathSequence's context
 * @param nfa_ptr pointer to the NFA to be constructed
 * @return antlrcpp::Any a dummy antlrcpp::Any object
 */
antlrcpp::Any
Rpq2NFAConvertor::visitPathSequence(rpqParser::PathSequenceContext *ctx, std::shared_ptr<NFA> nfa_ptr)
{
    if ((ctx->pathElt()).size() == 1)
        visitPathElt(ctx->pathElt(0), nfa_ptr);
    else
    {
        shared_ptr<NFA> currNfa_ptr;
        for (auto pathEltOrInverse : ctx->pathElt())
        {
            currNfa_ptr = make_shared<NFA>();
            visitPathElt(pathEltOrInverse, currNfa_ptr);
            // Attach transition from current accepts to sub-initial (label eps)
            for (auto acceptState : nfa_ptr->accepts)
                acceptState->addTransition(-1, true, currNfa_ptr->initial);
            nfa_ptr->unsetAccept();
            nfa_ptr->setAccept(currNfa_ptr->accepts);
            nfa_ptr->addStates(currNfa_ptr->states);
        }
    }
    return antlrcpp::Any();
}

/**
 * @brief pathElt : pathPrimary pathMod? ;
 * 
 * @param ctx pointer to pathElt's context
 * @param nfa_ptr pointer to the NFA to be constructed
 * @return antlrcpp::Any a dummy antlrcpp::Any object
 */
antlrcpp::Any
Rpq2NFAConvertor::visitPathElt(rpqParser::PathEltContext *ctx,
    std::shared_ptr<NFA> nfa_ptr)
{
    visitPathPrimary(ctx->pathPrimary(), nfa_ptr);
    if (ctx->pathMod())
    {
        string elt = ctx->pathMod()->getText();
        if (elt == "*" || elt == "+") {
            // Backward edge of Kleene star
            for (auto acceptState : nfa_ptr->accepts)
                acceptState->addTransition(-1, true, nfa_ptr->initial);
        }
        if (elt != "+")
            nfa_ptr->setAccept(nfa_ptr->initial);    // Both ? and *
    }
    return antlrcpp::Any();
}

/**
 * @brief pathPrimary : iri | '(' path ')' ;
 * iri : '[' num_integer '-'? ']' ;
 * 
 * @param ctx pointer to pathPrimary's context
 * @param nfa_ptr pointer to the NFA to be constructed
 * @return antlrcpp::Any a dummy antlrcpp::Any object
 */
antlrcpp::Any
Rpq2NFAConvertor::visitPathPrimary(rpqParser::PathPrimaryContext *ctx,
    std::shared_ptr<NFA> nfa_ptr)
{
    if (ctx->path())
        visitPath(ctx->path(), nfa_ptr);
    else if (ctx->iri())
    {
        int lbl_ = stoi(ctx->iri()->num_integer()->getText());
        bool forward_ = true;
        string iriStr = ctx->iri()->getText();
        if (iriStr.length() > 2 && iriStr[iriStr.length() - 2] == '-')
            forward_ = false;
        
        nfa_ptr->unsetAccept();
        shared_ptr<State> newAccept = nfa_ptr->addState(true); // TODO: addState
        nfa_ptr->initial->addTransition(lbl_, forward_, newAccept);
        nfa_ptr->setAccept(newAccept);
    }
    return antlrcpp::Any();
}