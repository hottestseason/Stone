module Stone
  class Parser
    class OperatorPriority
      attr_reader :value, :left_assoc

      def initialize(value, left_assoc)
        @value = value; @left_assoc = left_assoc
      end

      def prior_to?(right_priority)
        if left_assoc
          value > right_priority.value
        else
          value >= right_priority.value
        end
      end
    end

    OPERATORS = {}
    OPERATORS["="] = OperatorPriority.new(1, false)
    OPERATORS["=="] = OperatorPriority.new(2, true)
    OPERATORS[">"] = OperatorPriority.new(2, true)
    OPERATORS["<"] = OperatorPriority.new(2, true)
    OPERATORS["+"] = OperatorPriority.new(3, true)
    OPERATORS["-"] = OperatorPriority.new(3, true)
    OPERATORS["*"] = OperatorPriority.new(4, true)
    OPERATORS["/"] = OperatorPriority.new(4, true)
    OPERATORS["%"] = OperatorPriority.new(4, true)

    def initialize(tokens)
      @tokens = tokens
    end

    # [ def | statement ] (";" | "\n")
    def program
      if next_token
        if next_token.is_identifier?(";", "\n")
          read_next
          NullStmnt.create
        else
          if next_token.is_identifier?("def")
            define
          else
            statement
          end.tap do
            read_next if next_token && next_token.is_identifier?(";", "\n")
          end
        end
      else
        nil
      end
    end

    # "def" IDENTIFIER param_list block
    def define
      read_next.must_be_identifier!("def")
      DefStmnt.create(ASTLeaf.new(read_next), param_list, block)
    end

    # "(" [ params ] ")"
    def param_list
      read_next.must_be_identifier!("(")
      if next_token.is_identifier?(")")
        ASTList.create
      else
        params
      end.tap do
        read_next.must_be_identifier!(")")
      end
    end

    # param { "," param }
    def params
      params = [param]
      while next_token.is_identifier?(",")
        params << param
      end
      ParameterList.create(*params)
    end

    # IDENTIFIER
    def param
      ASTLeaf.create(read_next)
    end

    # "if" expression block [ "else" block ]
    # | "while" expression block
    # | simple
    def statement
      if next_token.is_identifier?("if")
        read_next
        condition = expression
        then_block = block
        if next_token.is_identifier?("else")
          read_next
          else_block = block
          IfStmnt.create(condition, then_block, else_block)
        else
          IfStmnt.create(condition, then_block)
        end
      elsif next_token.is_identifier?("while")
        read_next
        WhileStmnt.create(expression, block)
      else
        simple
      end
    end

    # expression [ args ]
    def simple
      children = [expression]
      while next_token && !next_token.is_identifier?(",", "\n")
        children << args
      end
      PrimaryStmnt.create(*children)
    end

    # "{" [ statement ] { (";" | "\n") [ statement ] } "}"
    def block
      statements = []
      read_next.must_be_identifier!("{")
      while !next_token.is_identifier?("}")
        statements << statement unless next_token.is_identifier?(";", "\n")
        while next_token.is_identifier?(";", "\n")
          read_next
          statements << statement unless next_token.is_identifier?("}")
        end
      end
      read_next.must_be_identifier!("}")
      BlockStmnt.create(*statements)
    end

    # factor { OP factor }
    def expression
      right = factor
      do_shift = -> (left, priority) {
        op = ASTLeaf.create(read_next)
        right = factor
        while (next_priority = OPERATORS[next_token.value]) && next_priority.prior_to?(priority)
          right = do_shift[right, next_priority]
        end
        BinaryExpr.create(left, op, right)
      }
      while next_token && priority = OPERATORS[next_token.value]
        right = do_shift[right, priority]
      end
      right
    end

    # "-" primary | primary
    def factor
      if next_token.is_identifier?("-")
        NegativeExpr.create(primary)
      else
        primary
      end
    end

    # ( "(" expr ")" | NUMBER | IDENTIFIER | STRING ) { postfix }
    def primary
      children = []
      children << if next_token && next_token.is_identifier?("(")
                    read_next
                    expression.tap do
                      next_token.must_be_identifier!(")")
                    end
                  elsif next_token.is_number?
                    NumberLiteral.create(read_next)
                  elsif next_token.is_identifier?
                    Name.create(read_next)
                  else
                    StringLiteral.create(read_next.must_be_string!)
                  end
      while next_token && next_token.is_identifier?("(")
        children << postfix
      end
      PrimaryStmnt.create(*children)
    end

    # "(" [ args ] ")"
    def postfix
      read_next.must_be_identifier!("(")
      if next_token.is_identifier?(")")
        ASTList.create
      else
        args
      end.tap do
        read_next.must_be_identifier!(")")
      end
    end

    # expr { "," expr }
    def args
      children = [expression]
      while next_token && next_token.is_identifier?(",")
        read_next
        children << expression
      end
      Arguments.create(*children)
    end

    private
    def next_token
      @tokens.first
    end

    def read_next
      @tokens.shift
    end
  end
end
