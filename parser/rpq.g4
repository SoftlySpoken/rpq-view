grammar rpq;

// export CLASSPATH="PATH-TO-antlr-4.7.2-complete.jar:$CLASSPATH"
// alias antlr4='java -Xmx500M -cp "PATH-TO-antlr-4.7.2-complete.jar:$CLASSPATH" org.antlr.v4.Tool'
// antlr4 -Dlanguage=Cpp ./SPARQL.g4 -visitor

// Parser rules

path : pathSequence ( '|' pathSequence )* ;
pathSequence : pathElt ( '/' pathElt )* ;
pathElt : pathPrimary pathMod? ;
pathMod : '?' | '*' | '+' ;
pathPrimary : iri | '(' path ')' ;
iri : '<' num_integer '-'? '>' ;
num_integer : INTEGER ;

// Lexer rules
INTEGER : [0-9]+ ;