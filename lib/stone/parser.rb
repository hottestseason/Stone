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

    # [ statement ] (";" | "\n")
    def program
      if next_token
        if next_token.is_identifier?(";", "\n")
          read_next
          NullStmnt.new
        else
          statement.tap do
            if next_token.is_identifier?(";", "\n")
              read_next
            end
          end
        end
      else
        nil
      end
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
          IfStmnt.new(condition, then_block, else_block)
        else
          IfStmnt.new(condition, then_block)
        end
      elsif next_token.is_identifier?("while")
        read_next
        WhileStmnt.new(expression, block)
      else
        simple
      end
    end

    # expression
    def simple
      expression
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
      BlockStmnt.new(*statements)
    end

    # factor { OP factor }
    def expression
      right = factor
      while next_token && priority = OPERATORS[next_token.value]
        right = do_shift(right, priority)
      end
      right
    end

    def do_shift(left, priority)
      op = ASTLeaf.new(read_next)
      right = factor
      while (next_priority = OPERATORS[next_token.value]) && next_priority.prior_to?(priority)
        right = do_shift(right, next_priority)
      end
      BinaryExpr.new(left, op, right)
    end

    # "-" primary | primary
    def factor
      if next_token.is_identifier?("-")
        NegativeExpr.new([primary])
      else
        primary
      end
    end

    # "(" expr ")" | NUMBER | IDENTIFIER | STRING
    def primary
      if next_token.is_identifier?("(")
        read_next
        expression.tap do
          next_token.must_be_identifier!(")")
        end
      elsif next_token.is_number?
        NumberLiteral.new(read_next)
      elsif next_token.is_identifier?
        Name.new(read_next)
      else
        StringLiteral.new(read_next.must_be_string!)
      end
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
