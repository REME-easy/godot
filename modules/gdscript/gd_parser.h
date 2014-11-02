/*************************************************************************/
/*  gd_parser.h                                                          */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                    http://www.godotengine.org                         */
/*************************************************************************/
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                 */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/
#ifndef GD_PARSER_H
#define GD_PARSER_H

#include "gd_tokenizer.h"
#include "gd_functions.h"
#include "map.h"
#include "object.h"

class GDParser {
public:

	struct Node {

		enum Type {
			TYPE_CLASS,
			TYPE_FUNCTION,
			TYPE_BUILT_IN_FUNCTION,
			TYPE_BLOCK,
			TYPE_IDENTIFIER,
			TYPE_TYPE,
			TYPE_CONSTANT,
			TYPE_ARRAY,
			TYPE_DICTIONARY,
			TYPE_SELF,
			TYPE_OPERATOR,
			TYPE_CONTROL_FLOW,
			TYPE_LOCAL_VAR,
			TYPE_ASSERT,
			TYPE_NEWLINE,
		};

		Node * next;
		int line;
		int column;
		Type type;

		virtual ~Node() {}
	};

	struct FunctionNode;
	struct BlockNode;

	struct ClassNode : public Node {

		bool tool;
		StringName name;
		bool extends_used;
		StringName extends_file;
		Vector<StringName> extends_class;

		struct Member {
			PropertyInfo _export;
#ifdef TOOLS_ENABLED
			Variant default_value;
#endif
			StringName identifier;
			StringName setter;
			StringName getter;
		};
		struct Constant {
			StringName identifier;
			Node *expression;
		};

		Vector<ClassNode*> subclasses;
		Vector<Member> variables;
		Vector<Constant> constant_expressions;
		Vector<FunctionNode*> functions;
		Vector<FunctionNode*> static_functions;
		BlockNode *initializer;
		//Vector<Node*> initializers;
		int end_line;

		ClassNode() { tool=false; type=TYPE_CLASS; extends_used=false; end_line=-1;}
	};



	struct FunctionNode : public Node {

		bool _static;
		StringName name;
		Vector<StringName> arguments;
		Vector<Node*> default_values;
		BlockNode *body;

		FunctionNode() { type=TYPE_FUNCTION; _static=false; }

	};

	struct BlockNode : public Node {

		Map<StringName,int> locals;
		List<Node*> statements;
		Vector<StringName> variables;
		Vector<int> variable_lines;

		//the following is useful for code completion
		List<BlockNode*> sub_blocks;
		int end_line;
		BlockNode() { type=TYPE_BLOCK; end_line=-1;}
	};

	struct TypeNode : public Node {

		Variant::Type vtype;
		TypeNode() { type=TYPE_TYPE;  }
	};
	struct BuiltInFunctionNode : public Node {
		GDFunctions::Function function;
		BuiltInFunctionNode() { type=TYPE_BUILT_IN_FUNCTION;  }
	};

	struct IdentifierNode : public Node {

		StringName name;
		IdentifierNode() { type=TYPE_IDENTIFIER;  }
	};

	struct LocalVarNode : public Node {

		StringName name;
		Node *assign;
		LocalVarNode() { type=TYPE_LOCAL_VAR;  assign=NULL;}
	};

	struct ConstantNode : public Node {
		Variant value;
		ConstantNode() { type=TYPE_CONSTANT; }
	};

	struct ArrayNode : public Node {

		Vector<Node*> elements;
		ArrayNode() { type=TYPE_ARRAY; }
	};


	struct DictionaryNode : public Node {

		struct Pair {

			Node *key;
			Node *value;
		};

		Vector<Pair> elements;
		DictionaryNode() { type=TYPE_DICTIONARY; }
	};

	struct SelfNode : public Node {
		SelfNode() { type=TYPE_SELF; }
	};

	struct OperatorNode : public Node {
		enum Operator {
			//call/constructor operator
			OP_CALL,
			OP_PARENT_CALL,
			OP_YIELD,
			OP_EXTENDS,
			//indexing operator
			OP_INDEX,
			OP_INDEX_NAMED,
			//unary operators
			OP_NEG,
			OP_NOT,
			OP_BIT_INVERT,
			OP_PREINC,
			OP_PREDEC,
			OP_INC,
			OP_DEC,
			//binary operators (in precedence order)
			OP_IN,
			OP_EQUAL,
			OP_NOT_EQUAL,
			OP_LESS,
			OP_LESS_EQUAL,
			OP_GREATER,
			OP_GREATER_EQUAL,
			OP_AND,
			OP_OR,
			OP_ADD,
			OP_SUB,
			OP_MUL,
			OP_DIV,
			OP_MOD,
			OP_SHIFT_LEFT,
			OP_SHIFT_RIGHT,
			OP_INIT_ASSIGN,
			OP_ASSIGN,
			OP_ASSIGN_ADD,
			OP_ASSIGN_SUB,
			OP_ASSIGN_MUL,
			OP_ASSIGN_DIV,
			OP_ASSIGN_MOD,
			OP_ASSIGN_SHIFT_LEFT,
			OP_ASSIGN_SHIFT_RIGHT,
			OP_ASSIGN_BIT_AND,
			OP_ASSIGN_BIT_OR,
			OP_ASSIGN_BIT_XOR,
			OP_BIT_AND,
			OP_BIT_OR,
			OP_BIT_XOR,
		};

		Operator op;

		Vector<Node*> arguments;
		OperatorNode() { type=TYPE_OPERATOR; }
	};

	struct ControlFlowNode : public Node {
		enum CFType {
			CF_IF,
			CF_FOR,
			CF_WHILE,
			CF_SWITCH,
			CF_BREAK,
			CF_CONTINUE,
			CF_RETURN
		};

		CFType cf_type;
		Vector<Node*> arguments;
		BlockNode *body;
		BlockNode *body_else;

		ControlFlowNode *_else; //used for if
		ControlFlowNode() { type=TYPE_CONTROL_FLOW; cf_type=CF_IF; body=NULL; body_else=NULL;}
	};

	struct AssertNode : public Node {
		Node* condition;
		AssertNode() { type=TYPE_ASSERT; }
	};

	struct NewLineNode : public Node {
		int line;
		NewLineNode() { type=TYPE_NEWLINE; }
	};


	struct Expression {

		bool is_op;
		union {
			OperatorNode::Operator op;
			Node *node;
		};
	};


/*
	struct OperatorNode : public Node {

		DataType return_cache;
		Operator op;
		Vector<Node*> arguments;
		virtual DataType get_datatype() const { return return_cache; }

		OperatorNode() { type=TYPE_OPERATOR; return_cache=TYPE_VOID; }
	};

	struct VariableNode : public Node {

		DataType datatype_cache;
		StringName name;
		virtual DataType get_datatype() const { return datatype_cache; }

		VariableNode() { type=TYPE_VARIABLE; datatype_cache=TYPE_VOID; }
	};

	struct ConstantNode : public Node {

		DataType datatype;
		Variant value;
		virtual DataType get_datatype() const { return datatype; }

		ConstantNode() { type=TYPE_CONSTANT; }
	};

	struct BlockNode : public Node {

		Map<StringName,DataType> variables;
		List<Node*> statements;
		BlockNode() { type=TYPE_BLOCK; }
	};

	struct ControlFlowNode : public Node {

		FlowOperation flow_op;
		Vector<Node*> statements;
		ControlFlowNode() { type=TYPE_CONTROL_FLOW; flow_op=FLOW_OP_IF;}
	};

	struct MemberNode : public Node {

		DataType datatype;
		StringName name;
		Node* owner;
		virtual DataType get_datatype() const { return datatype; }
		MemberNode() { type=TYPE_MEMBER; }
	};


	struct ProgramNode : public Node {

		struct Function {
			StringName name;
			FunctionNode*function;
		};

		Map<StringName,DataType> builtin_variables;
		Map<StringName,DataType> preexisting_variables;

		Vector<Function> functions;
		BlockNode *body;

		ProgramNode() { type=TYPE_PROGRAM; }
	};
*/


private:


	GDTokenizer *tokenizer;


	Node *head;
	Node *list;
	template<class T>
	T* alloc_node();

	bool validating;
	int parenthesis;
	bool error_set;
	String error;
	int error_line;
	int error_column;

	int pending_newline;

	List<int> tab_level;

	String base_path;

	PropertyInfo current_export;

	void _set_error(const String& p_error, int p_line=-1, int p_column=-1);


	bool _parse_arguments(Node* p_parent,Vector<Node*>& p_args,bool p_static);
	bool _enter_indent_block(BlockNode *p_block=NULL);
	bool _parse_newline();
	Node* _parse_expression(Node *p_parent,bool p_static,bool p_allow_assign=false);
	Node* _reduce_expression(Node *p_node,bool p_to_const=false);
	Node* _parse_and_reduce_expression(Node *p_parent,bool p_static,bool p_reduce_const=false,bool p_allow_assign=false);

	void _parse_block(BlockNode *p_block,bool p_static);
	void _parse_extends(ClassNode *p_class);
	void _parse_class(ClassNode *p_class);
	bool _end_statement();

	Error _parse(const String& p_base_path);

public:

	String get_error() const;
	int get_error_line() const;
	int get_error_column() const;
	Error parse(const String& p_code, const String& p_base_path="", bool p_just_validate=false);
	Error parse_bytecode(const Vector<uint8_t> &p_bytecode,const String& p_base_path="");

	const Node *get_parse_tree() const;

	void clear();
	GDParser();
	~GDParser();
};

#endif // PARSER_H
