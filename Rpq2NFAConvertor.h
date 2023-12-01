/**
 * @file Rpq2NFAConvertor.h
 * @author Yue Pang 
 * @brief Converts RPQ to NFA
 * @date 2022-08-24
 */

#pragma once

#include "Util.h"
#include "NFA.h"

class Rpq2NFAConvertor: public rpqBaseVisitor
{
public:
    void getClauses(const std::string &query, std::vector<std::string> &v);
    std::shared_ptr<NFA> convert(const std::string &query);	// Overall driver function
    antlrcpp::Any visitPath(rpqParser::PathContext *ctx, std::shared_ptr<NFA> nfa_ptr);
    antlrcpp::Any visitPathSequence(rpqParser::PathSequenceContext *ctx,
        std::shared_ptr<NFA> nfa_ptr);
    antlrcpp::Any visitPathElt(rpqParser::PathEltContext *ctx,
        std::shared_ptr<NFA> nfa_ptr);
    antlrcpp::Any visitPathPrimary(rpqParser::PathPrimaryContext *ctx,
        std::shared_ptr<NFA> nfa_ptr);
private:
    void getClausesSinglePath(rpqParser::PathContext *path, std::vector<std::string> &v);
};