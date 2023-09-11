#include "Util.h"

/**
	Throw a runtime error when lexical and syntactic errors are detected in the query.
*/
void RpqErrorListener::syntaxError(antlr4::Recognizer *recognizer, antlr4::Token * offendingSymbol, \
	size_t line, size_t charPositionInLine, const std::string &msg, std::exception_ptr e)
{
	throw std::runtime_error("[Syntax Error]:line " + std::to_string(line) + ":" \
        + std::to_string(charPositionInLine) + " " + msg);
}