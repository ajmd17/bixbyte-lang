#pragma once

#include <string>
#include <sstream>
#include <map>

#include <shared/source_location.hpp>

namespace bcparse {
  enum ErrorLevel {
    LEVEL_INFO,
    LEVEL_WARN,
    LEVEL_ERROR
  };

  enum ErrorMessage {
    /* Fatal errors */
    Msg_internal_error,
    Msg_custom_error,
    Msg_illegal_syntax,
    Msg_illegal_expression,
    Msg_illegal_operator,
    Msg_invalid_operator_for_type,
    Msg_const_modified,
    Msg_cannot_modify_rvalue,
    Msg_prohibited_action_attribute,
    Msg_unbalanced_expression,
    Msg_unexpected_character,
    Msg_unexpected_identifier,
    Msg_unexpected_token,
    Msg_unexpected_eof,
    Msg_unexpected_eol,
    Msg_unrecognized_escape_sequence,
    Msg_unterminated_string_literal,
    Msg_argument_after_varargs,
    Msg_too_many_args,
    Msg_too_few_args,
    Msg_arg_type_incompatible,
    Msg_named_arg_not_found,
    Msg_redeclared_identifier,
    Msg_redeclared_identifier_module,
    Msg_redeclared_identifier_type,
    Msg_undeclared_identifier,
    Msg_expected_identifier,
    Msg_keyword_cannot_be_used_as_identifier,
    Msg_ambiguous_identifier,
    Msg_invalid_constructor,
    Msg_expected_type_got_identifier,
    Msg_missing_type_and_assignment,
    Msg_type_no_default_assignment,

    /* FUNCTIONS */
    Msg_multiple_return_types,
    Msg_mismatched_return_type,
    Msg_must_be_explicitly_marked_any,
    Msg_any_reserved_for_parameters,
    Msg_return_outside_function,
    Msg_yield_outside_function,
    Msg_yield_outside_generator_function,
    Msg_not_a_function,
    Msg_member_not_a_method,
    Msg_closure_capture_must_be_parameter,
    Msg_pure_function_scope,

    /* ARRAYS */
    Msg_not_an_array,

    /* TYPES */
    Msg_undefined_type,
    Msg_redefined_type,
    Msg_redefined_builtin_type,
    Msg_type_not_defined_globally,
    Msg_identifier_is_type,
    Msg_mismatched_types,
    Msg_implicit_any_mismatch,
    Msg_type_not_generic,
    Msg_generic_parameters_missing,
    Msg_generic_parameter_redeclared,
    Msg_not_a_data_member,

    Msg_bitwise_operands_must_be_int,
    Msg_bitwise_operand_must_be_int,
    Msg_expected_token,
    Msg_unknown_opcode,
    Msg_unknown_directive,
    Msg_unknown_module,
    Msg_expected_module,
    Msg_empty_module,
    Msg_module_already_defined,
    Msg_module_not_imported,
    Msg_invalid_module_access,
    Msg_statement_outside_module,
    Msg_module_declared_in_block,
    Msg_could_not_open_file,
    Msg_could_not_find_module,
    Msg_identifier_is_module,
    Msg_import_outside_global,
    Msg_import_current_file,
    Msg_self_outside_class,
    Msg_else_outside_if,
    Msg_alias_missing_assignment,
    Msg_alias_must_be_identifier,
    Msg_unrecognized_alias_type,
    Msg_type_contract_outside_definition,
    Msg_unknown_type_contract_requirement,
    Msg_invalid_type_contract_operator,
    Msg_unsatisfied_type_contract,
    Msg_unsupported_feature,

    Msg_unreachable_code,
    Msg_expected_end_of_statement,

    /* Info */
    Msg_unused_identifier,
    Msg_empty_function_body,
    Msg_empty_statement_body,
    Msg_module_name_begins_lowercase
  };

  class CompilerError {
    static const std::map<ErrorMessage, std::string> error_message_strings;

  public:
    template <typename...Args>
    CompilerError(ErrorLevel level,
      ErrorMessage msg,
      const SourceLocation &location,
      const Args &...args)
        : m_level(level),
          m_msg(msg),
          m_location(location) {
      if (m_msg == Msg_custom_error) {
        makeMessage(args...);
      } else {
        std::string msg_str = error_message_strings.at(m_msg);
        makeMessage(msg_str.c_str(), args...);
      }
    }

    CompilerError(const CompilerError &other);
    ~CompilerError() {}

    inline ErrorLevel getLevel() const { return m_level; }
    inline ErrorMessage getMessage() const { return m_msg; }
    inline const SourceLocation &getLocation() const { return m_location; }
    inline const std::string &getText() const { return m_text; }

    bool operator<(const CompilerError &other) const;

  private:
    void makeMessage(const char *format) { m_text += format; }


    template <typename ... Args>
    void makeMessage(Args && ... args) {
      makeMessage(args...);
    }

    template <typename T, typename ... Args>
    void makeMessage(const char *format, T value, Args && ... args) {
      for (; *format; format++) {
        if (*format == '%') {
          std::stringstream sstream;
          sstream << value;

          m_text += sstream.str();

          makeMessage(format + 1, args...);

          return;
        }

        m_text += *format;
      }
    }

    ErrorLevel m_level;
    ErrorMessage m_msg;
    SourceLocation m_location;
    std::string m_text;
  };
}
