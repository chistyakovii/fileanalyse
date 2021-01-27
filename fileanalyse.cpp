#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define BUFSIZE 4096

enum lexeme_type {
	NOTYPE,
	NUMBER,
	STRINGCONST,
	LABEL,
	FUNCTION,
	VARIABLE,
	KEYWORD,
	DELIMITER,
};

enum { 
	NOERROR,
	ERROR,
};

struct item {
	char symb;
	struct item *next;
};


void output_char_list(struct item *char_list)
{
	while(char_list!=NULL) {
		putchar(char_list->symb);
		char_list=char_list->next;
	}
}

struct lexemes {
	char *word;
	int type;
	int string_number;
	struct lexemes *next; 
};

void delete_lexeme_list(struct lexemes **lex_list)
{
	while((*lex_list)!=NULL) {
		struct lexemes *lex_tmp=*lex_list;
		*lex_list=(*lex_list)->next;
		delete lex_tmp;
	}
}

struct errors {
	int number;
	int string_number;
	char *word;
};

int find_table_character(char character, const char *table)
{
	int i=0;
	while(table[i]!=0) {
		if(table[i]==character) {
			return 0;
		}
		i++;
	}
	return -1;
}

int find_table_string(char *string, const char **table)
{
	int i=0;
	while(table[i]!=NULL) {
		if(strcmp(string,table[i])==0) {
			return 0;
		}
		i++;
	}
	return -1;
}

char *return_word(char *string)
{
	char *word;
	int i, size;
	size=strlen(string);
	word=new char [size+1];
	for(i=0;i<size;i++) {
		word[i]=string[i];
	}
	word[i]='\0';
	return word;
}

		

void check_parameters(int argc)
{
	if(argc!=2) {
		printf("Bad number of parameters\n");
		exit(1);
	}
}

FILE* open_file(char **argv)
{
	FILE* fd=fopen(argv[1],"r");
	if(fd==NULL) {
		perror(argv[1]);
		exit(1);
	}
	return fd;
}

class Lexical_Analysis {

	static const char *key_table[];
	static const char delim_table[];
	static const char *function_table[];

	enum states {
		NUM,
		STRING,
		FLV,
		KEY,
		DELIM,
		INIT,
	};

	enum errortypes {
		NOERROR,
		ERROR,
		KEYWORD_ERR,
		WRONGSYMB_ERR,
		WRONGSTATE_ERR,
		NAME_FUNC_ERR,
	};


	int lex_type;
	int state;
	struct errors error;
	int string_number;
	struct item *char_list;
	struct lexemes *lexeme;

	void fill_error(int error_number, int str_number, char *word);
	void delete_char_list();
	void create_char_list(char symb);
	int get_item_len(struct item *char_list);
	void create_lexeme();
	void add_lexeme(char symb);
	struct lexemes* check_ready_or_add_lexeme(char symb);
	struct lexemes* number_state(char symb);
	struct lexemes* string_state(char symb);
	struct lexemes* label_func_var_states(char symb);
	struct lexemes* key_state(char symb);
	struct lexemes* delim_state(char symb);
	struct lexemes* init_state(char symb);


	void analyse_error();
public:
	Lexical_Analysis();
	/*Func in feed_character*/
	struct lexemes* feed_character(char symb);	
	int check_lexeme_list(struct lexemes *lex_list);
	int check_error_state();

};

const char* Lexical_Analysis::key_table[]={"if","then","goto","print",
			"buy","sell","prod","build","endturn",NULL};

const char* Lexical_Analysis::function_table[]={"my_id","turn","players",
			"active_players","supply","raw_price", "demand",
			"production_price","money","raw","production",
			"factories","auto_factories","manufactured",
			"result_raw_sold","result_raw_price",
			"result_prod_bought","result_prod_price",NULL};
 
const char Lexical_Analysis::delim_table[]={'+','-','*','/','%','<','>',
				':',',',';','(',')','=','[',']','!','{','}',
				'|','&'};

Lexical_Analysis::Lexical_Analysis()
{
	lex_type=NOTYPE;
	error.number=NOERROR;
	error.word=NULL;
	string_number=1;
	state=INIT;
	char_list=NULL;
	lexeme=NULL;
}

void Lexical_Analysis::fill_error(int error_number, int str_number, 
								char *word)
{
	error.number=error_number;
	error.string_number=str_number;
	error.word=word;
}

void Lexical_Analysis::delete_char_list()
{
	struct item *char_tmp;
	while(char_list!=NULL) {
		char_tmp=char_list;
		char_list=char_list->next;
		delete char_tmp;
	}
}

void Lexical_Analysis::create_char_list(char symb)
{
	static struct item *char_tmp;
	if(char_list==NULL) {
		char_list=new struct item;
		char_tmp=char_list;
	} else {
		char_tmp->next=new struct item;
		char_tmp=char_tmp->next;
	}
	char_tmp->symb=symb;
	char_tmp->next=NULL;
}

int Lexical_Analysis::get_item_len(struct item *char_list)
{
	int i=0;
	while(char_list!=NULL) {
		i++;
		char_list=char_list->next;
	}
	return i;
}

void Lexical_Analysis::create_lexeme()
{
	lexeme=new struct lexemes;
	lexeme->type=lex_type;
	lexeme->string_number=string_number;
	lexeme->word=new char [get_item_len(char_list)+1];
	int i=0;
	struct item *char_tmp=char_list;
	while(char_tmp!=NULL) {
		lexeme->word[i]=char_tmp->symb;
		i++;
		char_tmp=char_tmp->next;
	}
	lexeme->word[i]='\0';
	lexeme->next=NULL;
	delete_char_list();
}

void Lexical_Analysis::add_lexeme(char symb)
{
	struct lexemes *lexeme_tmp=lexeme;
	lexeme_tmp->next=new struct lexemes;
	lexeme_tmp=lexeme_tmp->next;
	lexeme_tmp->type=lex_type;
	lexeme_tmp->string_number=string_number;
	lexeme_tmp->word=new char [2];
	lexeme_tmp->word[0]=symb;
	lexeme_tmp->word[1]='\0';
	lexeme_tmp->next=NULL;
}
		
struct lexemes* Lexical_Analysis::check_ready_or_add_lexeme(char symb)
{
	if(symb==' '||symb=='\n'||symb=='\t') {
		create_lexeme();
		state=INIT;
		lex_type=NOTYPE;
		if(symb=='\n') {
			string_number++;
		}
		return lexeme;
	}
	if(find_table_character(symb,delim_table)==0) {
		create_lexeme();
		state=DELIM;
		lex_type=DELIMITER;
		add_lexeme(symb);
		state=INIT;
		lex_type=NOTYPE;
		return lexeme;
	}
	return NULL;
}

struct lexemes* Lexical_Analysis::number_state(char symb)
{
	if(symb>='0'&&symb<='9') {
		create_char_list(symb);
		return NULL;
	}
	if(check_ready_or_add_lexeme(symb)!=NULL) {
		return lexeme;
	}
	fill_error(WRONGSYMB_ERR,string_number,NULL);
	return NULL;
}

struct lexemes* Lexical_Analysis::key_state(char symb)
{
	if((symb>='a'&&symb<='z')||
	(symb>='A'&&symb<='Z')) {
		create_char_list(symb);
		return NULL;
	}
	if(check_ready_or_add_lexeme(symb)!=NULL) {
		return lexeme;
	}
	fill_error(WRONGSYMB_ERR,string_number,NULL);
	return NULL;
}

struct lexemes* Lexical_Analysis::label_func_var_states(char symb)
{
	if((symb>='0'&&symb<='9')||
	(symb>='a'&&symb<='z')||
	(symb>='A'&&symb<='Z')
	||symb=='_') {
		create_char_list(symb);
		return NULL;
	}
	if(check_ready_or_add_lexeme(symb)!=NULL) {
		return lexeme;
	}
	fill_error(WRONGSYMB_ERR,string_number,NULL);
	return NULL;
}

struct lexemes* Lexical_Analysis::string_state(char symb)
{
	if(symb!='"') {
		create_char_list(symb);
		return NULL;
	} else {
		create_lexeme();
		state=INIT;
		lex_type=NOTYPE;
		return lexeme;
	}
}

struct lexemes* Lexical_Analysis::delim_state(char symb)
{
	create_char_list(symb);
	create_lexeme();
	state=INIT;
	lex_type=NOTYPE;
	return lexeme;
}

struct lexemes* Lexical_Analysis::init_state(char symb) 
{
	if(symb>='0'&&symb<='9') {
		create_char_list(symb);
		state=NUM;
		lex_type=NUMBER;
		return NULL;
	}
	if(symb=='"') {
		state=STRING;
		lex_type=STRINGCONST;
		return NULL;
	}
	if(symb=='?') {
		state=FLV;
		lex_type=FUNCTION;
		return NULL;
	}
	if(symb=='@') {	
		state=FLV;
		lex_type=LABEL;
		return NULL;
	}
	if(symb=='$') {
		state=FLV;
		lex_type=VARIABLE;
		return NULL;
	}
	if((symb>='a'&&symb<='z')||
	(symb>='A'&&symb<='Z')) {
		create_char_list(symb);
		state=KEY;
		lex_type=KEYWORD;
		return NULL;
	}
	if(symb==' '||symb=='\t'||symb=='\n') {
		if(symb=='\n') {
			string_number++;
		}
		return NULL;
	}
	if(find_table_character(symb,delim_table)==0) {
		create_char_list(symb);
		state=DELIM;
		lex_type=DELIMITER;
		create_lexeme();
		state=INIT;
		lex_type=NOTYPE;
		return lexeme;
	}
	fill_error(WRONGSYMB_ERR,string_number,NULL);
	return NULL;
}

struct lexemes* Lexical_Analysis::feed_character(char symb)
{
	switch (state) {
		case NUM:
			return number_state(symb);
		case STRING:
			return string_state(symb);
		case FLV:
			return label_func_var_states(symb);
		case KEY:
			return key_state(symb);
		case DELIM:
			return delim_state(symb);
		case INIT:
			return init_state(symb);
		default:
			fill_error(WRONGSYMB_ERR,string_number,NULL);
			return NULL;
	}
}


int Lexical_Analysis::check_lexeme_list(struct lexemes *lex_list)
{
	while(lex_list!=NULL) {
		if(lex_list->type==KEYWORD) {
			if(find_table_string(lex_list->word,key_table)!=0) {
				fill_error(KEYWORD_ERR,lex_list->string_number,
						return_word(lex_list->word));
				return ERROR;
			}
		} else
		if(lex_list->type==FUNCTION) {	
			if(find_table_string(lex_list->word,
						function_table)!=0) {
				fill_error(NAME_FUNC_ERR,
						lex_list->string_number,
						return_word(lex_list->word));
				return ERROR;
			}
		}
		lex_list=lex_list->next;
	}
	return NOERROR;
}


void Lexical_Analysis::analyse_error()
{
	if(error.number==WRONGSYMB_ERR) {
		fprintf(stderr,
			"Error in text has occured in string number < %d >\n",
						error.string_number);
	} else
	if(error.number==KEYWORD_ERR) {
		fprintf(stderr,
			"Error has occured in the key word "
			"< %s > in string number < %d >\n",
				error.word, error.string_number);
	} else
	if(error.number==WRONGSTATE_ERR) {
		fprintf(stderr,
			"Error has occured in the state "
			"in string number < %d >\n",
					error.string_number);
	} else
	if(error.number==NAME_FUNC_ERR) {
		fprintf(stderr,
			"Error has occured in the function "
			"< %s > in string number < %d >\n",
					error.word, error.string_number);
	}
}

int Lexical_Analysis::check_error_state()
{
	if(error.number!=NOERROR) {
		analyse_error();
		return ERROR;
	} 
	return NOERROR;
}

void create_lexeme_list(struct lexemes *lexeme, struct lexemes **lex_list)
{
	static struct lexemes *lex_tmp;
	if(*lex_list==NULL) {
		*lex_list=lexeme;
		lex_tmp=*lex_list;
	} else {
		lex_tmp->next=lexeme;
	}
	while(lex_tmp->next!=NULL) {
		lex_tmp=lex_tmp->next;
	}

}

void output_lexeme_list(struct lexemes *lex_list)
{
	printf("================Lexeme list=============\n");
	while(lex_list!=NULL) {
		printf("%s",lex_list->word);
		if(lex_list->type==NUMBER) {
			printf("_____NUMBER");
		}
		if(lex_list->type==STRINGCONST) {
			printf("_____STRING CONSTANT");
		}
		if(lex_list->type==LABEL) {
			printf("_____LABEL");
		}
		if(lex_list->type==VARIABLE) {
			printf("_____VARIABLE");
		}
		if(lex_list->type==FUNCTION) {
			printf("_____FUNCTION");
		}
		if(lex_list->type==KEYWORD) {
			printf("_____KEY WORD");
		}
		if(lex_list->type==DELIMITER) {
			printf("_____DELIMITER");
		}
		printf("____string number = %d\n",
				lex_list->string_number);
		lex_list=lex_list->next;
	}
	printf("========================================\n");
}

struct lexemes *lexical_analysis(FILE* fd)
{
	Lexical_Analysis LA;
	struct lexemes *lexeme_list=NULL;
	int symb;
	while((symb=fgetc(fd))!=EOF) {
		struct lexemes *lex=LA.feed_character(symb);
		if(lex!=NULL) {
			create_lexeme_list(lex,&lexeme_list);
		} else {
			if(LA.check_error_state()==ERROR) {
				exit(1);
			}
		}
	}
	if(LA.check_lexeme_list(lexeme_list)==ERROR) {
		LA.check_error_state();
		exit(1);
	}
	return lexeme_list;
}










enum ParsingErrors {
	STATEMENT_ERR,
	SEMICOLON_ERR,
	GAME_NAME_ERR,
	GAME_OPERATOR_ERR,
	KEY_WORD_ERR,
	NULL_LIST_ERR,
	PRINT_ERR,
	OPERAND_ERR,
	SQUARE_BRACKET_ERR,
	DOUBLE_EQUAL_SIGN_ERR,
	EQUAL_SIGN_ERR,
	IF_ERR,
	THEN_ERR,
	LABEL_ERR,
	GOTO_ERR,
	VARIABLE_ERR,
	DELIMITER_ERR,
	RIGHT_BRACKET_ERR,
	LEFT_BRACKET_ERR,
	FUNCTION_ERR,
	AMPERSAND_ERR,
	EXPRESSION_BRACKET_ERR,
	NEGATION_ERR,
	EXISTED_LABEL_ERR,
	NOT_EXISTED_LABEL_ERR,
	NOT_DECLARED_LABEL_ERR,
	LEFT_BRACE_ERR,
};
	

class ParsingException {
	char *lexeme;
	int string_number;
	int parse_error;
public:
	const char *get_error_lexeme() const { return lexeme; };
	const int get_error_string() const { return string_number; };
	const char *get_error_msg() const;
	ParsingException(struct lexemes *lex, int error);
	ParsingException(const ParsingException& other);
	~ParsingException();
private:
	char *strdup(const char *string) const;
};


char *ParsingException::strdup(const char *string) const
{
	char *word;
	int i, size;
	size=strlen(string);
	word=new (char)(size+1);
	for(i=0;i<size;i++) {
		word[i]=string[i];
	}
	word[i]='\0';
	return word;
}

ParsingException::ParsingException(struct lexemes *lex, int error) 
{
	lexeme=strdup(lex->word);
	string_number=lex->string_number;
	parse_error=error;
}

ParsingException::ParsingException(const ParsingException& other) 
{

	lexeme=strdup(other.lexeme);
	string_number=other.string_number;
	parse_error=other.parse_error;
}

ParsingException::~ParsingException() 
{
	delete[] lexeme;
}

const char *ParsingException::get_error_msg() const
{
	if(lexeme!=NULL) {
		char error_msg[50];
		switch (parse_error) {
			case STATEMENT_ERR:
				sprintf(error_msg,"expected operator");
				break;
				break;
			case SEMICOLON_ERR:
				sprintf(error_msg,"before the lexeme "
						"expected < ; >");
				break;
			case GAME_NAME_ERR:
				sprintf(error_msg,"expected game key word");
				break;
			case GAME_OPERATOR_ERR:
				sprintf(error_msg,"expected game operator");
				break;
			case KEY_WORD_ERR:
				sprintf(error_msg,"expected key word");
				break;
			case NULL_LIST_ERR:
				sprintf(error_msg,"after the lexeme the list "
			       			"is null");
				break;
			case PRINT_ERR:
				sprintf(error_msg,"expected print");
				break;
			case OPERAND_ERR:
				sprintf(error_msg,"expected operand");
				break;
			case SQUARE_BRACKET_ERR:
				sprintf(error_msg,"expected square bracket");
				break;
			case EQUAL_SIGN_ERR:
				sprintf(error_msg,"expected sign < = >");
				break;
			case IF_ERR:
				sprintf(error_msg,"expected key word < if >");
				break;
			case THEN_ERR:
				sprintf(error_msg,"expected key word < then >");
				break;
			case LABEL_ERR:
				sprintf(error_msg,"expected label");
				break;
			case GOTO_ERR:
				sprintf(error_msg,"expected key word < goto >");
				break;
			case VARIABLE_ERR:
				sprintf(error_msg,"expected variable");
				break;
			case DOUBLE_EQUAL_SIGN_ERR:
				sprintf(error_msg,"expected < == >");
				break;
			case DELIMITER_ERR:
				sprintf(error_msg,"expected < | >");
				break;
			case RIGHT_BRACKET_ERR:
				sprintf(error_msg,"expected < ( >");
				break;
			case LEFT_BRACKET_ERR:
				sprintf(error_msg,"expected < ) >");
				break;
			case FUNCTION_ERR:
				sprintf(error_msg,"expected function");
				break;
			case AMPERSAND_ERR:
				sprintf(error_msg,"expected < & >");
				break;
			case EXPRESSION_BRACKET_ERR:
				sprintf(error_msg,"expected < ) > after " 
							"the expression");
				break;
			case NEGATION_ERR:
				sprintf(error_msg,"expected < = > after " 
							"the negation");
				break;
			case EXISTED_LABEL_ERR:
				sprintf(error_msg,"this label "
						"already existed");
				break;
			case NOT_EXISTED_LABEL_ERR:
				sprintf(error_msg,"couldn`t jump "
						"using this label");
				break;
			case NOT_DECLARED_LABEL_ERR:
				sprintf(error_msg,"the label not declared");
				break;
			case LEFT_BRACE_ERR:
				sprintf(error_msg,"expected < { > after then");
				break;

		}
		return strdup(error_msg);
	} else {
		const char list_over[]="Lexeme list was over";
		return strdup(list_over);
	}
}


class RPNElem;
class RPNLabel;
class RPNSysLabel;
class RPNInt;
class RPNVarAddr;


//=====================Classes that are in RPNElem========================//

class RPNEx {
	RPNElem *elem;
public:
	virtual void set_elem(RPNElem *element) {
		elem=element;
	}
	RPNElem *get_elem() {
		return elem;
	}
	virtual ~RPNEx() {}
};

class RPNExNotInt: public RPNEx {
public:
	RPNExNotInt(RPNElem *elem) {
		set_elem(elem);
	}
};

class RPNExNotVar: public RPNEx {
public:
	RPNExNotVar(RPNElem *elem) {
		set_elem(elem);
	}
};

class RPNExNotStringInt: public RPNEx {
public:
	RPNExNotStringInt(RPNElem *elem) {
		set_elem(elem);
	}
};

class RPNExNotLabel: public RPNEx {
public:
	RPNExNotLabel(RPNElem *elem) {
		set_elem(elem);
	}
};

class RPNExNotSysLabel: public RPNEx {
public:
	RPNExNotSysLabel(RPNElem *elem) {
		set_elem(elem);
	}
};


class RPNExNotVarTable: public RPNEx {
public:
	RPNExNotVarTable(RPNElem *elem) {
		set_elem(elem);
	}
};

class RPNExNotVarAddrVar: public RPNEx {
public:
	RPNExNotVarAddrVar(RPNElem *elem) {
		set_elem(elem);
	}
};


struct RPNItem;

class RPNElem {
public:
	virtual ~RPNElem() {}
	virtual void Evaluate(RPNItem **stack, 
				RPNItem **curr_elem) const = 0;
	virtual void RPNPrint() const = 0;

protected:
	static void push(RPNItem **stack, RPNElem *elem);
	static RPNElem *pop(RPNItem **stack);
};

struct RPNItem {
	int item_number;
	RPNElem *elem;
	RPNItem *next;
};

enum SerchInTables {
	FOUND,
	NOTFOUND,
	NOTDECLARED,
	DECLARED,
};

struct LabelItem {
	RPNItem **value;
	char *label_name;
	int declaration;
	struct LabelItem *next;
};

class LabelArrTable {
	struct LabelItem *label_table;
	struct LabelItem *label_last_table;
public:
	LabelArrTable() {
		label_table=NULL;
	}
	~LabelArrTable() {
		while(label_table!=NULL) {
			struct LabelItem *tmp=label_table;
			label_table=label_table->next;
			delete tmp;
		}
	}
	char *check_linked_labels() {
		struct LabelItem *tmp=label_table;
		while(tmp!=NULL) {
			if(tmp->declaration==NOTDECLARED) {
				return tmp->label_name;
			}
			tmp=tmp->next;
		}
		return NULL;
	}
	int check_label_in_table(char *name) {
		struct LabelItem *tmp=label_table;
		while(tmp!=NULL) {
			if(strcmp(tmp->label_name,name)==0) {
				tmp->declaration=DECLARED;
				return FOUND;
			}
			tmp=tmp->next;
		}
		return NOTFOUND;
	}
	RPNItem *get_label_addr(char *name) {
		struct LabelItem *tmp=label_table;
		while(tmp!=NULL) {
			if(strcmp(tmp->label_name,name)==0) {
				return *(tmp->value);
			}
			tmp=tmp->next;
		}
		return NULL;
	}
	void add_label_in_table(RPNItem **value, char *name) {
		if(label_table==NULL) {
			label_table=new LabelItem;
			label_last_table=label_table;
		} else {
			label_last_table->next=new LabelItem;
			label_last_table=label_last_table->next;
		}
		label_last_table->value=value;
		label_last_table->declaration=NOTDECLARED;
		label_last_table->label_name=new (char)(strlen(name));
		int i=0;
		while(name[i]!='\0') {
			label_last_table->label_name[i]=name[i];
			i++;
		}
		label_last_table->label_name[i]='\0';
		label_last_table->next=NULL;
	}
	void output_label_table() {
		struct LabelItem *tmp=label_table;
		while(tmp!=NULL) {
			printf("Label name < %s > \n", tmp->label_name);
			tmp=tmp->next;
		}
	}

};

struct VarItem {
	int value;
	char *var_name;
	struct VarItem *next;
};

class VarArrTable {
	struct VarItem *var_table;
	struct VarItem *var_last_table;
public:
	VarArrTable() {
		var_table=NULL;
	}
	~VarArrTable() {
		while(var_table!=NULL) {
			struct VarItem *tmp=var_table;
			var_table=var_table->next;
			delete tmp;
		}
	}
	int check_var_in_table(char *name) {
		struct VarItem *tmp=var_table;
		while(tmp!=NULL) {
			if(strcmp(tmp->var_name,name)==0) {
				return FOUND;
			}
			tmp=tmp->next;
		}
		return NOTFOUND;
	}
	int get_value_from_table(char *name) {
		struct VarItem *tmp=var_table;
		while(tmp!=NULL) {
			if(strcmp(tmp->var_name,name)==0) {
				return tmp->value;
			}
			tmp=tmp->next;
		}
		return -1;
	}
	int set_value_in_table(char *name,int value) {
		struct VarItem *tmp=var_table;
		while(tmp!=NULL) {
			if(strcmp(tmp->var_name,name)==0) {
				tmp->value=value;
			}
			tmp=tmp->next;
		} 
		return ERROR;
	}
	void add_var_in_table(int value, char *name) {
		VarItem *tmp_table=var_table;
		while(tmp_table!=NULL) {
			if(strcmp(tmp_table->var_name,name)==0) {
				tmp_table->value=value;
				return;
			}
			tmp_table=tmp_table->next;
		}
		if(var_table==NULL) {
			var_table=new VarItem;
			var_last_table=var_table;
		} else {
			var_last_table->next=new VarItem;
			var_last_table=var_last_table->next;
		}
		var_last_table->value=value;
		var_last_table->var_name=new (char)(strlen(name));
		int i=0;
		while(name[i]!='\0') {
			var_last_table->var_name[i]=name[i];
			i++;
		}
		var_last_table->var_name[i]='\0';
		var_last_table->next=NULL;
	}
	void output_var_table() {
		struct VarItem *tmp_table=var_table;
		while(tmp_table!=NULL) {
			printf("var_name=%s var_value=%d\n",
					tmp_table->var_name,tmp_table->value);
			tmp_table=tmp_table->next;
		}
	}
};

void RPNElem::push(RPNItem **stack,RPNElem *elem)
{
	RPNItem *tmp=new RPNItem;
	tmp->elem=elem;
	tmp->next=*stack;
	*stack=tmp;
}

RPNElem* RPNElem::pop(RPNItem **stack)
{
	RPNElem *tmp;
	tmp=(*stack)->elem;
	(*stack)=(*stack)->next;
	return tmp;
}

class RPNConst: public RPNElem {
public:
	virtual RPNElem *Clone() const = 0;
	virtual void Evaluate(struct RPNItem **stack,
			struct RPNItem **curr_elem) const {
		push(stack, Clone());
		(*curr_elem)=(*curr_elem)->next;
	}
};

class RPNFunction : public RPNElem {
public:
	virtual ~RPNFunction() {}
	virtual RPNElem *EvaluateFun(RPNItem **stack) const = 0;
	virtual void Evaluate(struct RPNItem **stack,
					struct RPNItem **curr_elem) const {
		RPNElem *res=EvaluateFun(stack);
		if(res!=NULL) {
			push(stack,res);
		}
		(*curr_elem)=(*curr_elem)->next;
	}
};

class RPNLabel: public RPNConst {
	RPNItem *value;
	char *label_name;
public:
	RPNLabel(RPNItem *a,char *string) {
		value=a;
		label_name=return_word(string);
	}
	virtual ~RPNLabel() {
		delete[] label_name;
	}
	virtual void RPNPrint() const {
		printf("%s point to item number < %d > RPNLabel\n",
				label_name, value->item_number);
	}
	RPNItem *Get() const {
		return value;
	}
	char *get_name() const {
		char *name=return_word(label_name);
		return name;
	}
	void set_addr(RPNItem *addr) {
		value=addr;
	}
	virtual RPNElem *Clone() const {
		return new RPNLabel(value,label_name);
	}
};

class RPNSysLabel: public RPNConst {
	RPNItem **value;
public:
	RPNSysLabel(RPNItem **a) {
		value=a;
	}
	virtual ~RPNSysLabel() {}
	virtual void RPNPrint() const {
		printf("point to item number < %d  > RPNSysLabel\n",
						(*value)->item_number);
	}
	RPNItem *Get() const {
		return *value;
	}
	virtual RPNElem *Clone() const {
		return new RPNSysLabel(value);
	}
};

class RPNOpGo: public RPNElem {
public:
	virtual void RPNPrint() const {
		printf("RPNOpGo\n");
	}
	virtual void Evaluate(struct RPNItem **stack,
			struct RPNItem **curr_elem) const {
		RPNElem *operand1=pop(stack);
		RPNLabel *label=dynamic_cast<RPNLabel*>(operand1);
		if(!label) {
			throw new RPNExNotLabel(operand1);
		}
		(*curr_elem)=label->Get();
		delete operand1;
	}
};

class RPNInt: public RPNConst {
	int value;
public:
	RPNInt(int a) { 
		value=a; 
	}
	virtual ~RPNInt() {}
	virtual void RPNPrint() const {
		printf("%d RPNInt\n",value);
	}
	int Get() const {
		return value;
	}
	virtual RPNElem *Clone() const {
		return new RPNInt(value);
	}
	
};

class RPNVarAddr: public RPNConst {
	VarArrTable *table;
	char *var_name;
	bool index;
public:
	RPNVarAddr(char *string,VarArrTable *t,bool ind) {
		table=t;
		var_name=return_word(string);
		index=ind;
	}
	virtual RPNElem *Clone() const {
		if(index==false) {
			return new RPNInt(Get());
		} else {
			return new RPNVarAddr(var_name,table,index);
		} 
	}
	char *get_name() {
		return var_name;
	}
	void change_var_addr_name(char *string) {
		delete[] var_name;
		var_name=return_word(string);
	}
	void set_index(bool ind) {
		index=ind;
	}
	virtual void RPNPrint() const {
		printf("%s RPNVarAddr\n",var_name);
	}
	virtual ~RPNVarAddr() {
		delete[] var_name;
	}
	int Get() const {
		if(table->check_var_in_table(var_name)==NOTFOUND) {
			throw new RPNExNotVarTable(new 
					RPNVarAddr(var_name,table,index));
		}
		return table->get_value_from_table(var_name);
	}
	VarArrTable *get_table() {
		return table;
	}
};

class RPNFunVar : public RPNFunction {
	VarArrTable *table;
	char *var_name;
public:
	RPNFunVar(char *string,VarArrTable *t) {
		table=t;
		var_name=return_word(string);
	}
	virtual ~RPNFunVar() {
		delete[] var_name;
	}
	virtual void RPNPrint() const {
		printf("%s RPNFunVar\n",var_name);
	}
	void set_value(int value) {
		if(table->check_var_in_table(var_name)==NOTFOUND) {
			table->add_var_in_table(value,var_name);
		}
		table->set_value_in_table(var_name,value);
	}
	char *get_name() {
		return var_name;
	}
	VarArrTable *get_table() {
		return table;
	}
	RPNElem *Clone() {
		return new RPNFunVar(var_name,table);
	}
	virtual RPNElem *EvaluateFun(RPNItem **stack) const {
		return new RPNFunVar(var_name,table);
	}
};

class RPNIndex: public RPNElem {
public:
	virtual void RPNPrint() const {
		printf("RPNIndex\n");
	}
	virtual void Evaluate(struct RPNItem **stack,
			struct RPNItem **curr_elem) const {
		RPNElem *operand1=pop(stack);
		RPNInt *i1=dynamic_cast<RPNInt*>(operand1);
		if(!i1) {
			throw new RPNExNotInt(operand1);
		}
		RPNElem *operand2=pop(stack);
		RPNFunVar *var=dynamic_cast<RPNFunVar*>(operand2);
		if(var) {
			int res=i1->Get();
			int size=1;
			while(res!=0) {
				res=res/10;
				size++;
			}
			size=size+strlen(var->get_name());
			char var_name[size+1];
			sprintf(var_name,"%s%d",var->get_name(),i1->Get());
			VarArrTable *table=var->get_table();
			push(stack,new RPNFunVar(var_name,table));
			delete operand1;
			delete operand2;		
		} else {
			RPNVarAddr *var=dynamic_cast<RPNVarAddr*>(operand2);
			if(!var) {
				throw RPNExNotVarAddrVar(operand2);
			}
			int res=i1->Get();
			int size=1;
			while(res!=0) {
				res=res/10;
				size++;
			}
			size=size+strlen(var->get_name());
			char var_name[size+1];
			sprintf(var_name,"%s%d",var->get_name(),i1->Get());
			var->change_var_addr_name(var_name);
			push(stack,new RPNInt(var->Get()));
			delete operand1;
			delete operand2;
		}
		(*curr_elem)=(*curr_elem)->next;
	}
};




class RPNOpGoFalse: public RPNElem {
public:
	virtual void RPNPrint() const {
		printf("RPNOpGoFalse\n");
	}
	virtual void Evaluate(struct RPNItem **stack,
			struct RPNItem **curr_elem) const {
		RPNElem *operand1=pop(stack);
		RPNSysLabel *label=dynamic_cast<RPNSysLabel*>(operand1);
		if(!label) {
			throw new RPNExNotSysLabel(operand1);
		}
		RPNElem *operand2=pop(stack);
		RPNInt *i1=dynamic_cast<RPNInt*>(operand2);
		if(!i1) {
			throw new RPNExNotInt(operand1);
		}
		if(i1->Get()>0) {
			(*curr_elem)=(*curr_elem)->next;
		} else {
			(*curr_elem)=label->Get();
		}
		delete operand1;
		delete operand2;
	}
};

//==========================Classes that are in RPNConst===================//



class RPNString: public RPNConst {
	char *value;
public:
	RPNString(char *string) {
		value=return_word(string);
	}
	virtual ~RPNString() {
		delete[] value;
	}
	virtual void RPNPrint() const {
		printf("%s RPNString\n",value);
	}
	char *Get() const {
		return value;
	}
	virtual RPNElem *Clone() const {
		return new RPNString(value);
	}
};









//===========================Classes that are in RPNFucntion===============//




class RPNFunPlus : public RPNFunction {
public:
	RPNFunPlus() {}
	virtual ~RPNFunPlus() {}
	virtual RPNElem *EvaluateFun(RPNItem **stack) const
	{
		RPNElem *operand1=pop(stack);
		RPNInt *i1=dynamic_cast<RPNInt*>(operand1);
		if(!i1) {
			throw new RPNExNotInt(operand1);
		}
		RPNElem *operand2=pop(stack);
		RPNInt *i2=dynamic_cast<RPNInt*>(operand2);
		if(!i2) {
			throw new RPNExNotInt(operand2);
		}
		int result=i1->Get()+i2->Get();
		delete operand1;
		delete operand2;
		return new RPNInt(result);
	}
	virtual void RPNPrint() const {
		printf("+ RPNFunPlus\n");
	}
};

class RPNFunMinus : public RPNFunction {
public:
	RPNFunMinus() {}
	virtual ~RPNFunMinus() {}
	virtual RPNElem *EvaluateFun(RPNItem **stack) const
	{
		RPNElem *operand1=pop(stack);
		RPNInt *i1=dynamic_cast<RPNInt*>(operand1);
		if(!i1) {
			throw new RPNExNotInt(operand1);
		}
		RPNElem *operand2=pop(stack);
		RPNInt *i2=dynamic_cast<RPNInt*>(operand2);
		if(!i2) {
			throw new RPNExNotInt(operand2);
		}
		int result=i2->Get()-i1->Get();
		delete operand1;
		delete operand2;
		return new RPNInt(result);
	}
	virtual void RPNPrint() const {
		printf("- RPNFunMinus\n");
	}
};

class RPNFunDivide: public RPNFunction{
public:
	virtual ~RPNFunDivide() {}
	virtual RPNElem *EvaluateFun(RPNItem **stack) const
	{
		RPNElem *operand1=pop(stack);
		RPNInt *i1=dynamic_cast<RPNInt*>(operand1);
		if(!i1) {
			throw new RPNExNotInt(operand1);
		}
		RPNElem *operand2=pop(stack);
		RPNInt *i2=dynamic_cast<RPNInt*>(operand2);
		if(!i2) {
			throw new RPNExNotInt(operand2);
		}
		int result=i2->Get()/i1->Get();
		delete operand1;
		delete operand2;
		return new RPNInt(result);
	}
	virtual void RPNPrint() const {
		printf("/ RPNFunDivide\n");
	}
};

class RPNFunMultiply: public RPNFunction {
public:
	virtual ~RPNFunMultiply() {}
	virtual RPNElem *EvaluateFun(RPNItem **stack) const {
		RPNElem *operand1=pop(stack);
		RPNInt *i1=dynamic_cast<RPNInt*>(operand1);
		if(!i1) {
			throw new RPNExNotInt(operand1);
		}
		RPNElem *operand2=pop(stack);
		RPNInt *i2=dynamic_cast<RPNInt*>(operand2);
		if(!i2) {
			throw new RPNExNotInt(operand2);
		}
		int result=i2->Get()*i1->Get();
		delete operand1;
		delete operand2;
		return new RPNInt(result);
	}
	virtual void RPNPrint() const {
		printf("* RPNFunMultiply\n");
	}
};

class RPNFunOr: public RPNFunction {
public:
	virtual ~RPNFunOr() {}
	virtual void RPNPrint() const {
		printf("|| RPNFunOr\n");
	}
	virtual RPNElem *EvaluateFun(struct RPNItem **stack) const {
		RPNElem *operand1=pop(stack);
		RPNInt *i1=dynamic_cast<RPNInt*>(operand1);
		if(!i1) {
			throw new RPNExNotInt(operand1);
		}
		RPNElem *operand2=pop(stack);
		RPNInt *i2=dynamic_cast<RPNInt*>(operand2);
		if(!i2) {
			throw new RPNExNotInt(operand2);
		}
		int res=i1->Get()||i2->Get();
		delete operand1;
		delete operand2;
		return new RPNInt(res);
	}		
};

class RPNFunAnd: public RPNFunction {
public:
	RPNFunAnd() {}
	virtual ~RPNFunAnd() {}
	virtual void RPNPrint() const {
		printf("&& RPNFunAnd\n");
	}
	virtual RPNElem *EvaluateFun(struct RPNItem **stack) const {
		RPNElem *operand1=pop(stack);
		RPNInt *i1=dynamic_cast<RPNInt*>(operand1);
		if(!i1) {
			throw new RPNExNotInt(operand1);
		}
		RPNElem *operand2=pop(stack);
		RPNInt *i2=dynamic_cast<RPNInt*>(operand2);
		if(!i2) {
			throw new RPNExNotInt(operand2);
		}
		int res=i1->Get()&&i2->Get();
		delete operand1;
		delete operand2;
		return new RPNInt(res);
	}		
};

class RPNFunLess : public RPNFunction {
public:
	virtual void RPNPrint() const {
		printf("<\n RPNFunLess\n");
	}
	virtual RPNElem *EvaluateFun(struct RPNItem **stack) const {
		RPNElem *operand1=pop(stack);
		RPNInt *i1=dynamic_cast<RPNInt*>(operand1);
		if(!i1) {
			throw new RPNExNotInt(operand1);
		}
		RPNElem *operand2=pop(stack);
		RPNInt *i2=dynamic_cast<RPNInt*>(operand2);
		if(!i2) {
			throw new RPNExNotInt(operand2);
		}
		int res;
		if(i2->Get()<i1->Get()) {
			res=1;
		} else {
			res=0;
		}
		delete operand1;
		delete operand2;
		return new RPNInt(res);
	}		
};

class RPNFunMore : public RPNFunction {
public:
	virtual void RPNPrint() const {
		printf("> RPNFunMore\n");
	}
	virtual RPNElem *EvaluateFun(struct RPNItem **stack) const {
		RPNElem *operand1=pop(stack);
		RPNInt *i1=dynamic_cast<RPNInt*>(operand1);
		if(!i1) {
			throw new RPNExNotInt(operand1);
		}
		RPNElem *operand2=pop(stack);
		RPNInt *i2=dynamic_cast<RPNInt*>(operand2);
		if(!i2) {
			throw new RPNExNotInt(operand2);
		}
		int res;
		if(i2->Get()>i1->Get()) {
			res=1;
		} else {
			res=0;
		}
		delete operand1;
		delete operand2;
		return new RPNInt(res);
	}		
};

class RPNFunEqual : public RPNFunction {
public:
	virtual void RPNPrint() const {
		printf("== RPNFunEqual\n");
	}
	virtual RPNElem *EvaluateFun(struct RPNItem **stack) const {
		RPNElem *operand1=pop(stack);
		RPNInt *i1=dynamic_cast<RPNInt*>(operand1);
		if(!i1) {
			throw new RPNExNotInt(operand1);
		}
		RPNElem *operand2=pop(stack);
		RPNInt *i2=dynamic_cast<RPNInt*>(operand2);
		if(!i2) {
			throw new RPNExNotInt(operand2);
		}
		int res;
		if(i2->Get()==i1->Get()) {
			res=1;
		} else {
			res=0;
		}
		delete operand1;
		delete operand2;
		return new RPNInt(res);
	}		
};

class RPNFunNot : public RPNFunction {
public:
	virtual void RPNPrint() const {
		printf("!= RPNFunNot\n");
	}
	virtual RPNElem *EvaluateFun(struct RPNItem **stack) const {
		RPNElem *operand1=pop(stack);
		RPNInt *i1=dynamic_cast<RPNInt*>(operand1);
		if(!i1) {
			throw new RPNExNotInt(operand1);
		}
		RPNElem *operand2=pop(stack);
		RPNInt *i2=dynamic_cast<RPNInt*>(operand2);
		if(!i2) {
			throw new RPNExNotInt(operand2);
		}
		int res;
		if(i2->Get()!=i1->Get()) {
			res=1;
		} else {
			res=0;
		}
		delete operand1;
		delete operand2;
		return new RPNInt(res);
	}		
};

class RPNFunAssignment : public RPNFunction {
public:
	virtual void RPNPrint() const {
		printf("= RPNFunAssignment\n");
	}
	virtual RPNElem *EvaluateFun(struct RPNItem **stack) const {
		RPNElem *operand1=pop(stack);
		RPNInt *i1=dynamic_cast<RPNInt*>(operand1);
		if(!i1) {
			throw new RPNExNotInt(operand1);
		}
		RPNElem *operand2=pop(stack);
		RPNFunVar *var=dynamic_cast<RPNFunVar*>(operand2);
		if(!var) {
			throw new RPNExNotVar(operand2);
		}
		var->set_value(i1->Get());
		delete operand1;
		delete operand2;
		return NULL;

	}
};

class RPNFunPrint : public RPNFunction {
public:
	virtual void RPNPrint() const {
		printf("print RPNFunPrint\n");
	}
	virtual RPNElem *EvaluateFun(struct RPNItem **stack) const {
		RPNElem *operand1=pop(stack);
		RPNInt *i1=dynamic_cast<RPNInt*>(operand1);
		if(!i1) {
			RPNString *i1=dynamic_cast<RPNString*>(operand1);
			if(!i1) { 
				throw new RPNExNotStringInt(operand1);
			} else {
				char *string=return_word(i1->Get());
				int size=strlen(string);
				if(string[size-2]=='\\'&&
				string[size-1]=='n') {
					string[size-2]='\0';
					printf("%s\n",string);
				} else {
					printf("%s",string);
				}
				delete[] string;
			}
		} else {
			printf("%d",i1->Get());
		}
		delete operand1;
		return NULL;
	}
};

class RPNFunGameNameZero : public RPNFunction {
	char *action;
public:
	RPNFunGameNameZero(char *string) {
		action=return_word(string);
	}
	virtual ~RPNFunGameNameZero() {
		delete[] action;
	}
	virtual void RPNPrint() const {
		printf("%s RPNFunGameNameZero\n",action);
	}
	virtual RPNElem *EvaluateFun(struct RPNItem **stack) const {
		return NULL;
	}
};

class RPNFunGameNameOne : public RPNFunction {
	char *action;
public:
	RPNFunGameNameOne(char *string) {
		action=return_word(string);
	}
	virtual ~RPNFunGameNameOne() {
		delete[] action;
	}
	virtual void RPNPrint() const {
		printf("%s RPNFunGameNameOne\n",action);
	}
	virtual RPNElem *EvaluateFun(struct RPNItem **stack) const {
		RPNElem *operand1=pop(stack);
		RPNInt *i1=dynamic_cast<RPNInt*>(operand1);
		if(!i1) {
			throw new RPNExNotInt(operand1);
		} 
		delete operand1;
		return NULL;
	}		
};

class RPNFunGameNameTwo : public RPNFunction {
	char *action;
public:
	RPNFunGameNameTwo(char *string) {
		action=return_word(string);
	}
	virtual ~RPNFunGameNameTwo() {
		delete[] action;
	}
	virtual void RPNPrint() const {
		printf("%s RPNFunGameNameTwo\n",action);
	}
	virtual RPNElem *EvaluateFun(struct RPNItem **stack) const {
		RPNElem *operand1=pop(stack);
		RPNInt *i1=dynamic_cast<RPNInt*>(operand1);
		if(!i1) {
			throw new RPNExNotInt(operand1);
		} 
		RPNElem *operand2=pop(stack);
		RPNInt *i2=dynamic_cast<RPNInt*>(operand2);
		if(!i2) {
			throw new RPNExNotInt(operand2);
		}
		delete operand1;
		delete operand2;
		return NULL;
	}		

};


class RPNFunServer : public RPNFunction {
	char *function_name;
	static const char *func_without_param[];
	static const char *func_with_param[]; 
public:
	RPNFunServer(char *string) {
		function_name=return_word(string);
	}
	virtual ~RPNFunServer() {
		delete[] function_name;
	}
	virtual void RPNPrint() const {
		printf("%s RPNFunServer\n",function_name);
	}
	virtual RPNElem *EvaluateFun(struct RPNItem **stack) const;

};

const char* RPNFunServer::func_without_param[]={"my_id", "turn", "players", 
		"active_players", "supply", "raw_price", 
		"demand", "production_price", NULL};

const char* RPNFunServer::func_with_param[]={"money", "raw", "prodcution", 
		"factories", 
		"auto_factories", "manufactured", "result_raw_sold",
		"result_row_price", "result_prod_bought",
		"result_prod_price", NULL};

RPNElem* RPNFunServer::EvaluateFun(struct RPNItem **stack) const 
{
	if(find_table_string(function_name,func_without_param)==0) {
		return NULL;
	} else 
	if(find_table_string(function_name,func_with_param)==0) {
		RPNElem *operand1=pop(stack);
		RPNInt *i1=dynamic_cast<RPNInt*>(operand1);
		if(!i1) {
			throw RPNExNotInt(operand1);
		}
		delete operand1;
		return NULL;

	} else {
		return NULL;
	}
}

//===========================================================================//


class Parsing {
	struct lexemes *lex_list;

	struct RPNItem *rpn_list;
	struct RPNItem *rpn_last_list;
	int item_number;
	VarArrTable *VT;
	LabelArrTable LT; 

	static const char *game_actions[];
	static const char *game_name_zero[];
	static const char *game_name_one[];
	static const char *game_name_two[];
public:
	Parsing(VarArrTable *VarTable) {
		VT=VarTable;
		item_number=0;
		rpn_list=new RPNItem;
		rpn_last_list=rpn_list;
	}
	RPNItem *parse_lexeme_list(struct lexemes *lexeme_list,
						VarArrTable *VarTable);
private:
	void next_link();
	void function_operator();
	void operand_operator();
	void expression_two_operator();
	void expression_one_operator();
	void expression_operator();
	void var_operator();
	void assignment_operator();
	void print_list_operator();
	void print_operator();
	void game_operator();
	void conditional_operator();
	void cycle_operator();
	void goto_operator();
	void statements();
	void block_statement();
	void statement();
	void grammar_purpose();


	void link_labels(); 
	void linking_error(char *name, int error);
	void set_index_in_var_addr();

	void create_rpn_list(RPNElem *elem);

};

const char* Parsing::game_actions[]={"endturn","prod","build",
						"buy","sell",NULL};
const char* Parsing::game_name_zero[]={"endturn",NULL};
const char* Parsing::game_name_one[]={"prod","build",NULL};
const char* Parsing::game_name_two[]={"buy","sell",NULL};

void Parsing::create_rpn_list(RPNElem *elem)
{
	item_number++;
	rpn_last_list->next=new RPNItem;
	rpn_last_list=rpn_last_list->next;
	rpn_last_list->elem=elem;
	rpn_last_list->item_number=item_number;
	rpn_last_list->next=NULL;
}

void Parsing::next_link()
{
	if(lex_list==NULL) {
		return;
	} else {
		if(lex_list->next==NULL) {
			if(strcmp(lex_list->word,";")!=0&&
				strcmp(lex_list->word,"}")!=0) {
				throw ParsingException(lex_list,NULL_LIST_ERR);
			}
		}
		lex_list=lex_list->next;
	}
}

void Parsing::function_operator()
{	if(lex_list->type==FUNCTION) {
		struct lexemes *tmp=lex_list;
		next_link();
		if(strcmp(lex_list->word,"(")!=0) {
			throw ParsingException(lex_list,RIGHT_BRACKET_ERR);
		}
		next_link();
		if(lex_list->type==NUMBER||lex_list->type==FUNCTION||
		lex_list->type==VARIABLE) {
			expression_operator();
		}
		if(strcmp(lex_list->word,")")!=0) {
			throw ParsingException(lex_list,LEFT_BRACKET_ERR);
		}
		next_link();
		create_rpn_list(new RPNFunServer(tmp->word));
	} else {
		throw ParsingException(lex_list,FUNCTION_ERR);
	}
}

void Parsing::operand_operator()
{
	if(lex_list->type==NUMBER) {
		create_rpn_list(new RPNInt(atoi(lex_list->word)));
		next_link();
	} else
	if(lex_list->type==VARIABLE) {
		create_rpn_list(new RPNVarAddr(lex_list->word,VT,false));
		var_operator();
	} else
	if(lex_list->type==FUNCTION) {
		function_operator();
	} else 
	if(strcmp(lex_list->word,"(")==0) {
		next_link();
		expression_operator();
		if(strcmp(lex_list->word,")")!=0) {
			throw ParsingException(lex_list,EXPRESSION_BRACKET_ERR);
		}
		next_link();
	} else {
		throw ParsingException(lex_list,OPERAND_ERR);
	}
}


void Parsing::expression_two_operator()
{
	operand_operator();
	while(strcmp(lex_list->word,"*")==0||
	strcmp(lex_list->word,"/")==0||
	strcmp(lex_list->word,"&")==0) {
		if(strcmp(lex_list->word,"*")==0) {
			next_link();
			operand_operator();
			create_rpn_list(new RPNFunMultiply());
		} else
		if(strcmp(lex_list->word,"/")==0) {
			next_link();
			operand_operator();
			create_rpn_list(new RPNFunDivide());
		} else 
		if(strcmp(lex_list->word,"&")==0) {
			next_link();
			if(strcmp(lex_list->word,"&")!=0) {
				throw ParsingException(lex_list,AMPERSAND_ERR);
			}
			next_link();
			operand_operator();
			create_rpn_list(new RPNFunAnd());
		}
	}
}

void Parsing::expression_one_operator()
{
	expression_two_operator();
	while(strcmp(lex_list->word,"+")==0||
	strcmp(lex_list->word,"-")==0||
	strcmp(lex_list->word,"|")==0) {
		if(strcmp(lex_list->word,"+")==0) {
			next_link();
			expression_two_operator();
			create_rpn_list(new RPNFunPlus());
		} else 
		if(strcmp(lex_list->word,"-")==0) {
			next_link();
			expression_two_operator();
			create_rpn_list(new RPNFunMinus());
		} else
		if(strcmp(lex_list->word,"|")==0) {
			next_link();
			if(strcmp(lex_list->word,"|")!=0) {
				throw ParsingException(lex_list,DELIMITER_ERR);
			}
			next_link();
			expression_two_operator();
			create_rpn_list(new RPNFunOr());
		}
	}
}

void Parsing::expression_operator()
{
	expression_one_operator();
	if(strcmp(lex_list->word,"=")==0) {
		next_link();
		if(strcmp(lex_list->word,"=")!=0) {
			throw ParsingException(lex_list,DOUBLE_EQUAL_SIGN_ERR);	
		}
		next_link();
		expression_one_operator();
		create_rpn_list(new RPNFunEqual());
	} else
	if(strcmp(lex_list->word,"<")==0) {
		next_link();
		expression_one_operator();
		create_rpn_list(new RPNFunLess());
	} else
	if(strcmp(lex_list->word,">")==0) {
		next_link();
		expression_one_operator();
		create_rpn_list(new RPNFunMore());
	} else
	if(strcmp(lex_list->word,"!")==0) {
		next_link();
		if(strcmp(lex_list->word,"=")!=0) {
			throw ParsingException(lex_list,NEGATION_ERR);
		}
		next_link();
		expression_one_operator();
		create_rpn_list(new RPNFunNot());
	}
}

void Parsing::var_operator()
{
	if(lex_list->type==VARIABLE) {
		next_link();
		if(strcmp(lex_list->word,"[")==0) {
			next_link();
			expression_operator();
			create_rpn_list(new RPNIndex);
			if(strcmp(lex_list->word,"]")!=0) {
				throw ParsingException(lex_list,
						SQUARE_BRACKET_ERR);
			}	
			next_link();
		}
	} else {
		throw ParsingException(lex_list,VARIABLE_ERR);
	}
}

void Parsing::assignment_operator()
{
	create_rpn_list(new RPNFunVar(lex_list->word,VT));
	var_operator();
	if(strcmp(lex_list->word,"=")==0) {
		next_link();
		expression_operator();
		create_rpn_list(new RPNFunAssignment());
	} else {
		throw ParsingException(lex_list,EQUAL_SIGN_ERR);
	}
}

void Parsing::print_list_operator() 
{
	if(lex_list->type==STRINGCONST) {
		create_rpn_list(new RPNString(lex_list->word));
		next_link();
	} else {
		expression_operator();
	}
	create_rpn_list(new RPNFunPrint());
	if(strcmp(lex_list->word,",")==0) {
		next_link();
		print_list_operator();
	}
}

void Parsing::print_operator()
{
	if(strcmp(lex_list->word,"print")==0) {
		next_link();
		print_list_operator();
	} else { 
		throw ParsingException(lex_list,PRINT_ERR);
	}
}

void Parsing::game_operator()
{
	if(lex_list->type==KEYWORD) {
		if(find_table_string(lex_list->word,game_name_zero)==0) {
			create_rpn_list(new 
					RPNFunGameNameZero(lex_list->word));
			next_link();
		} else
		if(find_table_string(lex_list->word,game_name_one)==0) {
			struct lexemes *tmp=lex_list;
			next_link();
			operand_operator();
			create_rpn_list(new 
					RPNFunGameNameOne(tmp->word));
		} else
		if(find_table_string(lex_list->word,game_name_two)==0) {
			struct lexemes *tmp=lex_list;
			next_link();
			operand_operator();
			operand_operator();
			create_rpn_list(new 
					RPNFunGameNameTwo(tmp->word));
		} else {
			throw ParsingException(lex_list,GAME_NAME_ERR);
		}
	} else {
		throw ParsingException(lex_list,KEY_WORD_ERR);
	}
}

void Parsing::conditional_operator()
{
	if(strcmp(lex_list->word,"if")==0) {
		next_link();
		expression_operator();
		create_rpn_list(NULL);
		RPNItem *tmp=rpn_last_list;
		create_rpn_list(new RPNOpGoFalse);
		if(strcmp(lex_list->word,"then")!=0) {
			throw ParsingException(lex_list,THEN_ERR);
		}
		next_link();
		if(strcmp(lex_list->word,"{")==0) {
			next_link();
			block_statement();
			next_link();
		} else {
			throw ParsingException(lex_list,LEFT_BRACE_ERR);
		}
		tmp->elem=new RPNSysLabel(&rpn_last_list->next);

	} else {
		throw ParsingException(lex_list,IF_ERR);
	}
}

void Parsing::cycle_operator()
{
	if(lex_list->type==LABEL) {
		if(LT.check_label_in_table(lex_list->word)==NOTFOUND) {
			LT.add_label_in_table(&rpn_last_list->next,
							lex_list->word);
		} else {
			throw ParsingException(lex_list,EXISTED_LABEL_ERR);
		}
	}
	next_link();
}

void Parsing::goto_operator()
{
	if(strcmp(lex_list->word,"goto")!=0) {
		throw ParsingException(lex_list,GOTO_ERR);
	}
	next_link();
	if(lex_list->type==LABEL) {
		create_rpn_list(new RPNLabel(NULL,
				lex_list->word));
		next_link();
	} else {
		throw ParsingException(lex_list,LABEL_ERR);
	}
	create_rpn_list(new RPNOpGo);
}

void Parsing::block_statement() 
{
	if(strcmp(lex_list->word,"}")!=0) {
		statement();
		block_statement();
	}
}


void Parsing::statement()
{
	if(lex_list->type==LABEL) {
		cycle_operator();
	} else 
	if(lex_list->type==KEYWORD&&strcmp(lex_list->word,"if")==0) {
		conditional_operator();
	} else {
		if(lex_list->type==VARIABLE) {
			assignment_operator();
		} else
		if(lex_list->type==KEYWORD) {
			if(strcmp(lex_list->word,"print")==0) {
				print_operator();
			} else 
			if(find_table_string(lex_list->word,
						game_actions)==0) {
				game_operator();
			} else
			if(strcmp(lex_list->word,"goto")==0) {
				goto_operator();
			} 
		} else {
			throw ParsingException(lex_list,STATEMENT_ERR);
		}
		if(strcmp(lex_list->word,";")!=0) {
			throw ParsingException(lex_list,SEMICOLON_ERR);
		}
		next_link();
	}
}

void Parsing::statements() {
	if(lex_list!=NULL) {
		statement();
		statements();
	}
}

void Parsing::grammar_purpose() {
	if(lex_list!=NULL) {
		if(lex_list->type==LABEL) {
			if(LT.check_label_in_table(lex_list->word)==NOTFOUND) {
				LT.add_label_in_table(&rpn_last_list->next,
							lex_list->word);
			} else {
				throw ParsingException(lex_list,
						EXISTED_LABEL_ERR);
			}
			next_link();
		}
		statement();
		statements();
	} 
}


void Parsing::linking_error(char *name, int error) 
{
	struct lexemes *lex_tmp=lex_list;
	while(lex_tmp!=NULL) {
		if(lex_tmp->type==LABEL&&
		strcmp(lex_tmp->word,name)==0) {
			break;
		}
		lex_tmp=lex_tmp->next;
	}
	delete[] name;
	throw ParsingException(lex_tmp,error);
}

void Parsing::link_labels() 
{
	struct RPNItem *rpn_tmp=rpn_list;
	while(rpn_tmp!=NULL) {
		RPNLabel *label=dynamic_cast<RPNLabel*>(rpn_tmp->elem);
		if(label) {
			char *name=label->get_name();
			if(LT.check_label_in_table(name)==NOTFOUND) {
				linking_error(name,NOT_EXISTED_LABEL_ERR);
			} else {
				label->set_addr(LT.get_label_addr(name));
			}
			delete[] name;
		}
		rpn_tmp=rpn_tmp->next;
	}
	char *name;
	if((name=LT.check_linked_labels())!=NULL)	{
		linking_error(name,NOT_DECLARED_LABEL_ERR);
	}
}

void Parsing::set_index_in_var_addr() 
{
	struct RPNItem *rpn_tmp=rpn_list;
	while(rpn_tmp!=NULL) {
		RPNVarAddr *var_addr=dynamic_cast<RPNVarAddr*>(rpn_tmp->elem);
		if(var_addr) {
			if(rpn_tmp->next!=NULL) {
				if(rpn_tmp->next->next!=NULL) {
					RPNIndex *index=
					dynamic_cast<RPNIndex*>
					(rpn_tmp->next->next->elem);
					if(index) {
						var_addr->set_index(true);
					}
				}
			}
		}
		rpn_tmp=rpn_tmp->next;
	}
}


RPNItem*  Parsing::parse_lexeme_list(struct lexemes *lexeme_list,
						VarArrTable *VarTable)
{
	VT=VarTable;
	lex_list=lexeme_list;
	if(lex_list!=NULL) {
		grammar_purpose();
	} else {
		printf("Lexeme list is empty\n");
	}
	lex_list=lexeme_list;
	rpn_list=rpn_list->next;
	link_labels();
	set_index_in_var_addr();
	return rpn_list;
}

void start_interpretation(struct RPNItem *rpn_list)
{
	struct RPNItem *stack=NULL, *curr_elem=rpn_list;
	while(curr_elem!=NULL) {
		//curr_elem->elem->RPNPrint();
		curr_elem->elem->Evaluate(&stack,&curr_elem);
	}
	printf("Interpretation was finished\n");
}

void find_got_elem(RPNEx *EX) 
{
	if(dynamic_cast<RPNOpGo*>(EX->get_elem())) {
		printf("but got RPNOpGo\n");
	} else 
	if(dynamic_cast<RPNOpGoFalse*>(EX->get_elem())) {
		printf("but got RPNOpGoFalse\n");
	} else 
	if(dynamic_cast<RPNInt*>(EX->get_elem())) {
		printf("but got RPNInt\n");
	} else 
	if(dynamic_cast<RPNString*>(EX->get_elem())) {
		printf("but got RPNString\n");
	} else 
	if(dynamic_cast<RPNLabel*>(EX->get_elem())) {
		printf("but got RPNLabel\n");
	} else 
	if(dynamic_cast<RPNSysLabel*>(EX->get_elem())) {
		printf("but got RPNSysLabel\n");
	} else 
	if(dynamic_cast<RPNVarAddr*>(EX->get_elem())) {
		printf("but got RPNVarAddr\n");
	} else 
	if(dynamic_cast<RPNFunVar*>(EX->get_elem())) {
		printf("but got RPNFunVar\n");
	} else 
	if(dynamic_cast<RPNFunPlus*>(EX->get_elem())) {
		printf("but got RPNFunPlus\n");
	} else 
	if(dynamic_cast<RPNFunMinus*>(EX->get_elem())) {
		printf("but got RPNFunMinus\n");
	} else 
	if(dynamic_cast<RPNFunDivide*>(EX->get_elem())) {
		printf("but got RPNFunDivide\n");
	} else 
	if(dynamic_cast<RPNFunMultiply*>(EX->get_elem())) {
		printf("but got RPNFunMultiply\n");
	} else 
	if(dynamic_cast<RPNFunOr*>(EX->get_elem())) {
		printf("but got RPNFunOr\n");
	} else 
	if(dynamic_cast<RPNFunAnd*>(EX->get_elem())) {
		printf("but got RPNFunAnd\n");
	} else 
	if(dynamic_cast<RPNFunLess*>(EX->get_elem())) {
		printf("but got RPNFunLess\n");
	} else 
	if(dynamic_cast<RPNFunMore*>(EX->get_elem())) {
		printf("but got RPNFunMore\n");
	} else 
	if(dynamic_cast<RPNFunEqual*>(EX->get_elem())) {
		printf("but got RPNFunEqual\n");
	} else 
	if(dynamic_cast<RPNFunNot*>(EX->get_elem())) {
		printf("but got RPNFunNot\n");
	} else 
	if(dynamic_cast<RPNFunAssignment*>(EX->get_elem())) {
		printf("but got RPNFunAssignment\n");
	} else 
	if(dynamic_cast<RPNFunPrint*>(EX->get_elem())) {
		printf("but got RPNFunPrint\n");
	} else 
	if(dynamic_cast<RPNFunGameNameZero*>(EX->get_elem())) {
		printf("but got RPNFunGameNameZero\n");
	} else 
	if(dynamic_cast<RPNFunGameNameOne*>(EX->get_elem())) {
		printf("but got RPNFunGameNameOne\n");
	} else 
	if(dynamic_cast<RPNFunGameNameTwo*>(EX->get_elem())) {
		printf("but got RPNFunGameNameTwo\n");
	} else 
	if(dynamic_cast<RPNFunServer*>(EX->get_elem())) {
		printf("but got RPNFunServer\n");
	} else  
	if(dynamic_cast<RPNIndex*>(EX->get_elem())) {
		printf("but got RPNIndex\n");
	} 
}	
	
void find_error_in_rpn(RPNEx *EX)
{
	if(dynamic_cast<RPNExNotInt*>(EX)) {
		printf("In RPN interpretation: " 
			"expected integer value ");
		find_got_elem(EX);
	} else
	if(dynamic_cast<RPNExNotStringInt*>(EX)) {
		printf("In RPN interpretation: " 
			"expected integer value or string constant ");
		find_got_elem(EX);
	} else 
	if(dynamic_cast<RPNExNotLabel*>(EX)) {
		printf("In RPN interpretation: " 
			"expected integer value ");
		find_got_elem(EX);
	} else 
	if(dynamic_cast<RPNExNotSysLabel*>(EX)) {
		printf("In RPN interpretation: " 
			"expected integer value ");
		find_got_elem(EX);
	} else 
	if(dynamic_cast<RPNExNotVar*>(EX)) {
		printf("In rpn interpretation: " 
			"expected variable ");
		find_got_elem(EX);
	} else 
	if(dynamic_cast<RPNExNotVarTable*>(EX)) {
		RPNVarAddr *i1=dynamic_cast<RPNVarAddr*>(EX->get_elem());
		printf("In rpn interpretation: " 
			"variable < %s > is not initialized\n",
						i1->get_name());
	} else 
	if(dynamic_cast<RPNExNotVarAddrVar*>(EX)) {
		printf("In rpn interpretation: " 
			"expected variable or variable address ");
		find_got_elem(EX);
	}
}

int main(int argc, char **argv)
{
	struct lexemes *lex_list;
	VarArrTable VarTable;
	Parsing PA(&VarTable);
	RPNItem *rpn_list;
	check_parameters(argc);
	FILE* fd=open_file(argv);
	if((lex_list=lexical_analysis(fd))==NULL) {
		printf("The list is empty\n");
		exit(0);
	}
	fclose(fd);
	//output_lexeme_list(lex_list);
	try {
		rpn_list=PA.parse_lexeme_list(lex_list,&VarTable);
		printf("The list successfully parsed\n");
		delete_lexeme_list(&lex_list);
		/*while(rpn_list!=NULL) {
			printf("%d) ",rpn_list->item_number);
			rpn_list->elem->RPNPrint();
			rpn_list=rpn_list->next;
		}*/
		start_interpretation(rpn_list);
	}
	catch(const ParsingException &PE) {
		fprintf(stderr,"Error has occured in lexeme < %s >, "
				"string number < %d >, %s\n",
				PE.get_error_lexeme(), PE.get_error_string(),
				PE.get_error_msg());
		exit(1);
	}
	catch(RPNEx *EX) {
		find_error_in_rpn(EX);
	}
	return 0;
}

