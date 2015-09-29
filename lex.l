fractional-part [0-9]*
exponent-part [eE][+-]?[0-9]+
floating-point-constant  (([0-9]+\.[0-9]*([eE][+-]?[0-9]+)?|\.[0-9]+([eE][+-]?[0-9]+)?))|([0-9]+\.)|([0-9]+([eE][+-]?[0-9]+))|([0-9]+\.([eE][+-]?[0-9]+))
integer-part [0-9]+
string \"((\\.)|[^"\\])*\"

%%
[ \t\n]+                                    // skip white space chars. 
{floating-point-constant}                  {Scanner::line_num =ScannerBase::lineNr();Parser::d_val__=string(matched()); return Parser::FLOAT_CONSTANT;}
{string} 	{Scanner::line_num =ScannerBase::lineNr();Parser::d_val__=string(matched());return Parser::STRING_LITERAL;}
void {Scanner::line_num =ScannerBase::lineNr();return Parser::VOID;}
int {Scanner::line_num =ScannerBase::lineNr();return Parser::INT;}
float {Scanner::line_num =ScannerBase::lineNr();return Parser::FLOAT;}
for {Scanner::line_num =ScannerBase::lineNr();return Parser::FOR;}
while {Scanner::line_num =ScannerBase::lineNr();return Parser::WHILE;}
if {Scanner::line_num =ScannerBase::lineNr();return Parser::IF;}
else {Scanner::line_num =ScannerBase::lineNr();return Parser::ELSE;}
return {Scanner::line_num =ScannerBase::lineNr();return Parser::RETURN;}
"!=" {Scanner::line_num =ScannerBase::lineNr();return Parser::NE_OP;}
"&&" {Scanner::line_num =ScannerBase::lineNr();return Parser::AND_OP;}
"||" {Scanner::line_num =ScannerBase::lineNr();return Parser::OR_OP;}
"<=" {Scanner::line_num =ScannerBase::lineNr();return Parser::LE_OP;}
">=" {Scanner::line_num =ScannerBase::lineNr();return Parser::GE_OP;}
"==" {Scanner::line_num =ScannerBase::lineNr();return Parser::EQ_OP;}
"++" {Scanner::line_num =ScannerBase::lineNr();return Parser::INC_OP;}
([1-9][0-9]*)|(0[bB][01]*)|(0[0-7]*)|(0[xX][0-9a-fA-F]*) {Parser::d_val__=string(matched());Scanner::line_num =ScannerBase::lineNr();return Parser::INT_CONSTANT;}
[[:alpha:]_][[:alpha:][:digit:]_]*          {Parser::d_val__=string(matched());Scanner::line_num =ScannerBase::lineNr();return Parser::IDENTIFIER;}
. return matched()[0];



