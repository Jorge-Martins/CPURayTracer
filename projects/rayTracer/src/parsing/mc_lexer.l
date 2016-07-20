%{
/* C++ string header, for string ops below */
#include <string>
/* Implementation of yyFlexScanner */ 
#include "mc_scanner.hpp"
#include <iostream>

/* typedef to make the returns for the tokens shorter */
typedef MC::MC_Parser::token token;

/* define to keep from re-typing the same code over and over */
#define STOKEN( x ) ( new std::string( x ) )

/* define yyterminate as this instead of NULL */
#define yyterminate() return(token::END)

/* msvc2010 requires that we exclude this header file. */
#define YY_NO_UNISTD_H

%}

%option debug 
%option nodefault 
%option yyclass="MC_Scanner" 
%option noyywrap 
%option c++

%%
\#.*         ; /* comment*/

"from"       {  return(token::VIEW_FROM);}
"at"         {  return(token::VIEW_AT);}
"up"         {  return(token::VIEW_UP);}
"angle"      {  return(token::VIEW_ANGLE);}
"hither"     {  return(token::VIEW_HITHER);}
"resolution" {  return(token::VIEW_RES);}

v           { return(token::VIEW);}
b           { return(token::BACK_COL);}
l           { return(token::LIGHT);}
f           { return(token::MATERIAL);}
c           { return(token::CYLINDER);}
s           { return(token::SPHERE);}
p           { return(token::POLY);}
"pp"        { return(token::POLY_PATCH);}
"pl"        { return(token::PLANE);}
"gpl"       { return(token::PLANE);}


[0-9]+      { 
              yylval->ival = std::stoi(std::string(yytext)); 
			  return (token::INT);
			}
[0-9]+(\.[0-9]+)?([eE][+-]?[0-9]+)? {
			  yylval->fval = std::stof(std::string(yytext));
			  return (token::FLOAT);
			}
\.[0-9]+([eE][+-]?[0-9]+)? {
			  yylval->fval = std::stof(std::string(yytext));
			  return (token::FLOAT);
			}
-           { return(token::NEGATIVE);}


[ \t\n]     ; /* ignore whitespace*/


.           std::cerr << "Unknown character" << std::endl;
%%


