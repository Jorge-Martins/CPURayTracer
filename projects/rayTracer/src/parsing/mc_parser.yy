%skeleton "lalr1.cc"
%require  "2.5"
%debug 
%defines 
%define namespace "MC"
%define parser_class_name "MC_Parser"

%code requires{
   namespace MC {
      class MC_Driver;
      class MC_Scanner;
   }
   #include "driver_structs.h"
   #include <vector>
}

%lex-param   { MC_Scanner  &scanner  }
%parse-param { MC_Scanner  &scanner  }

%lex-param   { MC_Driver  &driver  }
%parse-param { MC_Driver  &driver  }

%code{
   #include <iostream>
   #include <cstdlib>
   #include <fstream>

   /* include for all driver functions */
   #include "mc_driver.hpp"
  
   /* this is silly, but I can't figure out a way around */
   static int yylex(MC::MC_Parser::semantic_type *yylval,
                    MC::MC_Scanner  &scanner,
                    MC::MC_Driver   &driver);
   
}

/* token types */
%union {
   int          ival;
   float        fval;
   MC::vec3     v3val;
   MC::ivec2    iv2val;
   std::vector<MC::vec3> *v3lst;
}

%token            END    0     "end of file"
%token            VIEW BACK_COL LIGHT MATERIAL CYLINDER SPHERE POLY POLY_PATCH PLANE
%token            VIEW_FROM VIEW_AT VIEW_UP VIEW_ANGLE VIEW_HITHER VIEW_RES
%token            NEGATIVE
%token  <ival>    INT
%token  <fval>    FLOAT

%type   <v3val>   from at up fvec3
%type   <iv2val>  res ivec2
%type   <fval>    angle hither real
%type   <v3lst>   fvec3_list


%%

nff_file : END | list END;

list
  : item
  | list item
  ;

item
  : view
  | back
  | light
  | material
  | cylinder
  | sphere
  | poly
  | poly_patch
  | plane
  ;

view
  : VIEW from at up angle hither res { driver.add_view($2,$3,$4,$5,$6,$7);}
  ;
back
  : BACK_COL fvec3                   { driver.add_back_col($2);}
  ;
light
  : LIGHT fvec3                      { driver.add_light($2);}
  | LIGHT fvec3 fvec3                { driver.add_light($2,$3);}
  ;
material
  : MATERIAL fvec3 real real real real real { driver.add_material($2,$3,$4,$5,$6,$7);}
  ;
cylinder
  : CYLINDER fvec3 real fvec3 real   { driver.add_cylinder($2,$3,$4,$5);}
  ;
sphere
  : SPHERE fvec3 real                { driver.add_sphere($2,$3);}
  ;
poly
  : POLY INT fvec3_list              { driver.add_poly($2,$3);}
  ;
poly_patch
  : POLY_PATCH INT fvec3_list        { driver.add_poly_patch($2,$3);}
  ;
plane
  : PLANE fvec3 fvec3 fvec3          { driver.add_plane($2,$3,$4);}
  ;

from
  : VIEW_FROM fvec3                  { $$ = $2;}
  ;
at
  : VIEW_AT fvec3                    { $$ = $2;}
  ;
up
  : VIEW_UP fvec3                    { $$ = $2;}
  ;
angle
  : VIEW_ANGLE real                  { $$ = $2;}
  ;
hither
  : VIEW_HITHER real                 { $$ = $2;}
  ;
res
  : VIEW_RES ivec2                   { $$ = $2;}
  ;


fvec3
  : real real real                   { MC::vec3 res; res.x=$1; res.y=$2; res.z=$3; 
                                       $$ = res;}
  ;
ivec2
  : INT INT                          { MC::ivec2 res; res.x=$1; res.y=$2;
                                       $$ = res;}
  ;

real
  : FLOAT                            { $$ = $1;}
  | NEGATIVE FLOAT                   { $$ = -($2);}
  | INT                              { $$ = static_cast<float>($1);}
  | NEGATIVE INT                     { $$ = static_cast<float>(-$2);}
  ;

fvec3_list
  : fvec3                            { $$ = new std::vector<MC::vec3>(1,$1);}
  | fvec3_list fvec3                 { $$ = $1; ($$)->push_back($2);}
  ;

%%


void MC::MC_Parser::error( const MC::MC_Parser::location_type &l,
                           const std::string &err_message)
{
   std::cerr << "Error: " << err_message << "\n"; 
}


/* include for access to scanner.yylex */
#include "mc_scanner.hpp"
static int yylex(MC::MC_Parser::semantic_type *yylval,
                 MC::MC_Scanner  &scanner,
                 MC::MC_Driver   &driver)
{
   return( scanner.yylex(yylval) );
}

