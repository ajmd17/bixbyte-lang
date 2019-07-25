#include <bcparse/compiler_error.hpp>

namespace bcparse {
  const std::map<ErrorMessage, std::string> CompilerError::error_message_strings {
    /* Fatal errors */
    { Msg_internal_error, "Internal error" },
    { Msg_custom_error, "%" },
    { Msg_illegal_syntax, "Illegal syntax" },
    { Msg_illegal_expression, "Illegal expression" },
    { Msg_illegal_operator, "Illegal usage of operator '%'" },
    { Msg_invalid_operator_for_type, "Operator '%' is not valid for type '%'" },
    { Msg_const_modified, "Cannot modify a constant" },
    { Msg_cannot_modify_rvalue, "The left hand side is not suitable for assignment" },
    { Msg_prohibited_action_attribute, "Attribute '%' prohibits this action" },
    { Msg_unbalanced_expression, "Unbalanced expression" },
    { Msg_unexpected_character, "Unexpected character '%'" },
    { Msg_unexpected_identifier, "Unexpected identifier '%'" },
    { Msg_unexpected_token, "Unexpected token '%'" },
    { Msg_unexpected_eof, "Unexpected end of file" },
    { Msg_unexpected_eol, "Unexpected end of line" },
    { Msg_unrecognized_escape_sequence, "Unrecognized escape sequence '%'" },
    { Msg_unterminated_string_literal, "Unterminated string quotes" },
    { Msg_argument_after_varargs, "Argument not allowed after '...'" },
    { Msg_too_many_args, "Too many arguments for function call: % required, found %" },
    { Msg_too_few_args, "Too few arguments for function call: % required, found %" },
    { Msg_arg_type_incompatible, "'%' cannot be passed to a function as '%'" },
    { Msg_named_arg_not_found, "Could not find a parameter named '%'" },
    { Msg_redeclared_identifier, "Identifier '%' has already been declared in this scope" },
    { Msg_redeclared_identifier_module, "'%' is the name of a module and cannot be used as an identifier" },
    { Msg_redeclared_identifier_type, "'%' is the name of a type and cannot be used as an identifier" },
    { Msg_undeclared_identifier, "'%' is not declared in module %" },
    { Msg_expected_identifier, "Expected an identifier" },
    { Msg_keyword_cannot_be_used_as_identifier, "Keyword '%' cannot be used as a name in this case" },
    { Msg_ambiguous_identifier, "Identifier '%' is ambiguous" },
    { Msg_invalid_constructor, "Invalid constructor" },
    { Msg_expected_type_got_identifier, "'%' is an identifier, expected a type" },
    { Msg_missing_type_and_assignment, "No type or assignment has been provided for '%'" },
    { Msg_type_no_default_assignment, "Type '%' has no default assignment" },
    { Msg_multiple_return_types, "Function has more than one possible return type" },
    { Msg_mismatched_return_type, "Function is marked to return '%', cannot return '%'" },
    { Msg_must_be_explicitly_marked_any, "Function must be explicitly marked to return 'Any'" },
    { Msg_any_reserved_for_parameters, "'Any' type is reserved for function parameters" },
    { Msg_return_outside_function, "'return' not allowed outside of function body" },
    { Msg_yield_outside_function, "'yield' not allowed outside of function body" },
    { Msg_yield_outside_generator_function, "'yield' only allowed within generator functions" },
    { Msg_not_a_function, "An object of type '%' is not callable as a function" },
    { Msg_member_not_a_method, "Data member '%' is not a method" },
    { Msg_closure_capture_must_be_parameter, "'%' was declared in a function above this one, and must be passed as a parameter to be captured" },
    { Msg_pure_function_scope, "variables declared from an outside scope may not be used in a pure function" },
    { Msg_not_an_array, "Subscript operator invalid on type '%'" },
    { Msg_undefined_type, "'%' is not a built-in or user-defined type" },
    { Msg_redefined_type, "Type '%' has already been defined in this module" },
    { Msg_redefined_builtin_type, "Cannot create type '%', it is a built-in type" },
    { Msg_type_not_defined_globally, "Type definitions are not allowed in local scopes" },
    { Msg_identifier_is_type, "'%' is the name of a type, expected an identifier" },
    { Msg_mismatched_types, "Mismatched types '%' and '%'" },
    { Msg_implicit_any_mismatch, "An explicit cast to '%' is required" },
    { Msg_type_not_generic, "Type '%' is not generic" },
    { Msg_generic_parameters_missing, "Generic type '%' requires % parameter(s)" },
    { Msg_generic_parameter_redeclared, "Generic parameter '%' already declared" },
    { Msg_not_a_data_member, "'%' is not a member of type '%'" },
    { Msg_bitwise_operands_must_be_int, "Bitwise operands must both be 'Int', got '%' and '%'" },
    { Msg_bitwise_operand_must_be_int, "Bitwise operand must be 'Int', got '%'" },
    { Msg_expected_token, "Expected '%'" },
    { Msg_unknown_directive, "'%' is not a built-in or user-defined directive" },
    { Msg_unknown_module, "'%' is not an imported module" },
    { Msg_expected_module, "Statement found outside of module" },
    { Msg_empty_module, "The module is empty" },
    { Msg_module_already_defined, "Module '%' was already defined or imported" },
    { Msg_module_not_imported, "Module '%' was not imported" },
    { Msg_identifier_is_module, "'%' is the name of a module, expected an identifier" },
    { Msg_invalid_module_access, "'%' is a module, expected an identifier or function call" },
    { Msg_statement_outside_module, "Statement outside of module" },
    { Msg_module_declared_in_block, "A module may not be declared within a conditional, loop or function" },
    { Msg_could_not_open_file, "Could not open file '%'" },
    { Msg_could_not_find_module, "Could not find module '%' in paths %" },
    { Msg_import_outside_global, "Import not allowed outside of global scope" },
    { Msg_import_current_file, "Attempt to import current file" },
    { Msg_self_outside_class, "'self' not allowed outside of a class" },
    { Msg_else_outside_if, "'else' not connected to an if statement" },
    { Msg_alias_missing_assignment, "Alias '%' must have an assignment" },
    { Msg_alias_must_be_identifier, "Alias '%' must reference an identifier" },
    { Msg_unrecognized_alias_type, "Only identifiers, types and module names may be aliased" },
    { Msg_type_contract_outside_definition, "Type contracts not allowed outside of function definitions" },
    { Msg_unknown_type_contract_requirement, "Unknown type contract requirement: '%'" },
    { Msg_invalid_type_contract_operator, "Invalid type contract operator '%'. Supported operators are '|' and '&'" },
    { Msg_unsatisfied_type_contract, "Type '%' does not satisfy type contract" },
    { Msg_unsupported_feature, "Unsupported feature" },

    /* Warnings */
    { Msg_unreachable_code, "Unreachable code detected" },
    { Msg_expected_end_of_statement, "End of statement expected (use a newline or semicolon to end a statement)" },

    /* Info */
    { Msg_unused_identifier, "'%' is not used" },
    { Msg_empty_function_body, "The function body of '%' is empty" },
    { Msg_empty_statement_body, "Loop or statement body is empty" },
    { Msg_module_name_begins_lowercase, "Module name '%' should begin with an uppercase character" },
  };

  CompilerError::CompilerError(const CompilerError &other)
    : m_level(other.m_level),
      m_msg(other.m_msg),
      m_location(other.m_location),
      m_text(other.m_text) {
  }

  bool CompilerError::operator<(const CompilerError &other) const {
    if (m_level == other.m_level) {
      return m_location < other.m_location;
    } else {
      return m_level < other.m_level;
    }
  }
}
