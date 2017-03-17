/* 
 * Concepts of Programming Languages: Project #1
 * 
 * LiYanHao 1400012849
 *
 * myparse.cpp - recursive lexical and syntax parser for self-designed language
 *
 * BNF Description: 
 *		<Code> -> <Statement><Code> | epsilon
 * 		<Statement> -> <Expression>; | while (<Expression>) <Expression>;
 *		<Expression> -> <Term> + <Expression> | <Term> - <Expression> | <Term>
 *		<Term> -> <Factor> * <Term> | <Factor> / <Term> | <Factor>
 * 		<Factor> -> <integer>
 * 
 */
#include <cstring>
#include <cstdio>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <queue>
using namespace std;

/*DECLARATION*/
struct token{
	string type;
	bool used;
	token(string _type){type = _type; used = false; }
};

vector<token> tokenstream;
queue<string> traverse;
int spidx = 0;

int TokenCode();
int TokenStatement();
int TokenExpression();
int TokenTerm();
int TokenFactor();
int lexparse(string raw);


/*CODE*/
/*LEX*/
int lexparse(string raw){
	int len = raw.length();
	int index = 0, errornum;
	while(index != len){
		if (raw[index] == 'w'){
			//token-while
			if (raw.substr(index, 5) != "while"){
				printf("IGNORED: unexpected charactor: %c at position: %d.\n", raw[index], index);
				index++;
				errornum++;
				continue;
			}
			index += 5;
			tokenstream.push_back(token("while"));
		}
		else if (isdigit(raw[index])){
			//token-integer
			string tmp = string(1, raw[index]);
			while(isdigit(raw[++index]))
				tmp += string(1, raw[index]);
			tokenstream.push_back(token(tmp));
		}
		else if (raw[index] == '(' || raw[index] == ')' || raw[index] == ';'
			|| raw[index] == '+' || raw[index] == '-' || raw[index] == '*' || raw[index] == '/' ){
			//token-brackets & semicolons & operators
			tokenstream.push_back(token(string(1, raw[index])));
			index++;
		}
		else{
			//exception handler
			index++;
			errornum++;
			printf("IGNORED: unexpected charactor: %c at position: %d.\n", raw[index], index);
		}
	}
	return errornum;
}

/*SYNTAX*/
int TokenCode(){
	traverse.push("<code>");
	if (spidx == tokenstream.size()){
		traverse.push("epsilon");
		return 0;
	}

	//recurse
	int res1 = TokenStatement();

	int res2 = TokenCode();

	return res1 | res2;
}

int TokenStatement(){
	traverse.push("<statment>");
	if (tokenstream[spidx].type == "while" && !tokenstream[spidx].used){
		traverse.push("while");
		tokenstream[spidx].used = true;

		//fix the brackets problem
		if (tokenstream[spidx+1].type == "(" && !tokenstream[spidx+1].used){
			tokenstream[spidx+1].used = true;
			spidx += 2;
		}
		else{
			printf("FIXED: expecting \"(\" after \"while\". Token position: %d\n", spidx);
			spidx++;
		}
		traverse.push("(");

		int res1 = TokenExpression();

		//fix the brackets problem
		if (tokenstream[spidx].type == ")" && !tokenstream[spidx].used){
			tokenstream[spidx].used = true;
			spidx++;
		}
		else{
			printf("FIXED: Expecting \")\" after \"while\". Token position: %d\n", spidx);
		}
		traverse.push(")");

		int res2 = TokenExpression();
		
		//fix the semicolon problem
		if (tokenstream[spidx].type == ";" && !tokenstream[spidx].used){
			tokenstream[spidx].used = true;
			spidx++;
		}
		else{ 
			printf("FIXED: Expecting \";\" after \"while\". Token position: %d\n", spidx);
		}
		traverse.push(";");

		return res1 | res2;
	}
	else{
		int res = TokenExpression();
		//printf("TokenStatement res: %d\n", res);
		if (tokenstream[spidx].type == ";" && !tokenstream[spidx].used){
			tokenstream[spidx].used = true;
			spidx++;
		}
		else{
			printf("FIXED: Expecting \";\" after \"while\". Token position: %d\n", spidx);
		}
		traverse.push(";");
		return res;
	}
}

int TokenExpression(){
	traverse.push("<Expression>");
	int res1 = TokenTerm();

	//FOLLOW SET
	if (spidx == tokenstream.size() 
		|| tokenstream[spidx].type[0] == ';'
		|| tokenstream[spidx].type[0] == ')'){
		traverse.push("epsilon");
		return 0;
	}

	//FIRST SET
	if (tokenstream[spidx].type == "+" && !tokenstream[spidx].used){
		traverse.push("+");
		tokenstream[spidx].used = true;
		spidx++;
	}
	else if (tokenstream[spidx].type == "-" && !tokenstream[spidx].used){
		traverse.push("-");
		tokenstream[spidx].used = true;
		spidx++;
	}
	else{
		printf("ERROR: Expecting operator - '+' or '-'. Token position: %d\n", spidx);
		exit(1);
	}

	int res2 = TokenExpression();

	return res1 | res2;
}

int TokenTerm(){
	traverse.push("<Token>");
	int res1 = TokenFactor();

	//FOLLOW SET
	if (spidx == tokenstream.size() 
		|| tokenstream[spidx].type[0] == '+' 
		|| tokenstream[spidx].type[0] == '-'
		|| tokenstream[spidx].type[0] == ';'
		|| tokenstream[spidx].type[0] == ')'){
		traverse.push("epsilon");
		return 0;
	}

	if (tokenstream[spidx].type == "*" && !tokenstream[spidx].used){
		traverse.push("*");
		tokenstream[spidx].used = true;
		spidx++;
	}
	else if (tokenstream[spidx].type == "/" && !tokenstream[spidx].used){
		traverse.push("/");
		tokenstream[spidx].used = true;
		spidx++;
	}
	else{
		printf("ERROR: Expecting operator - '*' or '/'. Token position: %d\n", spidx);
		exit(1);
	}

	int res2 = TokenTerm();

	return res1 | res2;
}

int TokenFactor(){
	traverse.push("<Factor>");
	if (tokenstream[spidx].type[0] > '9' || tokenstream[spidx].type[0] < '0'){
		printf("ERROR: Expecting number. Token position: %d\n", spidx);
		exit(1);
	}
	traverse.push(tokenstream[spidx].type);
	spidx++;
	return 0;
}

//
int main(int argc, char* argv[]){
	while(!traverse.empty())
		traverse.pop();
	/*Part1: read from file*/
	string code;
	if (argc != 2){
		//define usage
		printf("Usage: \"myparser filepath\". Example: \"myparser ./sample.mylang\".\n");
		return 1;
	}
	freopen(argv[1], "r", stdin);
	char ch;
	while(scanf("%c", &ch) != EOF){
		//ignore delimiter
		if (ch == '\r' || ch == '\t' || ch == '\n' || ch == ' ')
			continue;
		code += string(1, ch);
	}

	/*Part2: lexical parse*/
	printf("Lexical Parsing. Printing messages.\n");
	int err = lexparse(code);
	if(err)
		printf("\nLEXICAL PARSE FAILED! Error Number: %d. Above error will be IGNORED.\n\n", err);
	else
		printf("\nLEXICAL PARSE SUCCEEDED!\n\n");

	printf("Printing token stream for debugging.\n");
	for (int i = 0; i < tokenstream.size(); ++i)
		printf("Token position: %d  Token type: %s\n", i, tokenstream[i].type.c_str());
	putchar('\n');

	/*Part3: syntax parse*/
	printf("Syntax Parsing. Printing messages.\n");
	if (TokenCode())
		printf("\nSYNTAX PARSE FAILED!\nSee details above.\n");
	else{
		printf("\nSYNTAX PARSE SUCCEEDED!\nPrinting depth-First traverse of the tree:\n");
		while(!traverse.empty()){
			printf("%s\n", traverse.front().c_str());
			traverse.pop();
		}
	}

	printf("\nThis is the END of my parser.\n");
	fclose(stdin);
	return 0;
}