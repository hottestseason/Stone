module Stone
  class ASTLeaf
    attr_reader :token

    def initialize(token)
      @token = token
    end

    def to_s
      token.value.to_s
    end
  end

  class NumberLiteral < ASTLeaf
    def eval(env)
      token.value
    end
  end

  class Name < ASTLeaf
    def eval(env)
      env[token.value]
    end
  end

  class StringLiteral < ASTLeaf
  end

  class ASTList
    attr_reader :children

    def initialize(*args)
      @children = args
    end

    def to_s
      children.map { |child| "(#{child})" }.join(" ")
    end
  end

  class BinaryExpr < ASTList
    def to_s
      "(#{left} #{operator} #{right})"
    end

    def left; children.first end
    def operator; children.second.token end
    def right; children.third end

    def eval(env)
      if operator.is_identifier?("=")
        name = left.token.value
        env[name] = right.eval(env)
      else
        Kernel.eval "#{left.eval(env)} #{operator.value} #{right.eval(env)}"
      end
    end
  end

  class IfStmnt < ASTList
    def to_s
      "(if #{condition} #{then_block} else #{else_block})"
    end

    def condition; children.first end
    def then_block; children.second end
    def else_block; children.third end

    def eval(env)
      if condition.eval(env)
        then_block.eval(env)
      else
        else_block.eval(env) if else_block
      end
    end
  end

  class WhileStmnt < ASTList
    def to_s
      "(while #{condition} #{body})"
    end

    def condition; children.first end
    def body; children.second end

    def eval(env)
      while condition.eval(env)
        body.eval(env)
      end
    end
  end

  class BlockStmnt < ASTList
    def eval(env)
      result = nil
      children.each do |child|
        result = child.eval(env)
      end
      result
    end
  end

  class NegativeExpr < ASTList
    def to_s
      "- #{operand}"
    end

    def operand; children.first end

    def eval(env)
      - operand.eval(env)
    end
  end

  class NullStmnt < ASTList
    def to_s
      ""
    end

    def eval(env)
    end
  end

  class DefStmnt < ASTList
    def to_s
      "(def #{name} #{parameters} #{body})"
    end

    def name
      children.first.token
    end

    def parameters
      children.second
    end

    def body
      children.third
    end
  end

  class ParameterList < ASTList
  end

  class PrimaryStmnt < ASTList
  end

  class Postfix < ASTList
  end

  class Arguments < Postfix
  end
end
