module Stone
  class ASTree
    extend Forwardable

    def self.create(*args)
      self.new(*args)
    end
  end

  class ASTLeaf < ASTree
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
      env.get(token.value)
    end
  end

  class StringLiteral < ASTLeaf
  end

  class ASTList < ASTree
    include Enumerable

    attr_reader :children

    def_delegators :children, :each, :second, :third

    def self.create(*args)
      self.new(args.flatten)
    end

    def initialize(children)
      @children = children
    end

    def to_s
      children.map { |child| "(#{child})" }.join(" ")
    end
  end

  class BinaryExpr < ASTList
    alias :left :first
    alias :right :third
    def_delegator :second, :token, :operator

    def to_s
      "(#{left} #{operator.value} #{right})"
    end

    def eval(env)
      if operator.is_identifier?("=")
        name = left.token.value
        env.create(name, right.eval(env))
      else
        Kernel.eval "#{left.eval(env)} #{operator.value} #{right.eval(env)}"
      end
    end
  end

  class IfStmnt < ASTList
    alias :condition :first
    alias :then_block :second
    alias :else_block :third

    def to_s
      "(if #{condition} #{then_block} else #{else_block})"
    end

    def eval(env)
      if condition.eval(env)
        then_block.eval(env)
      else
        else_block.eval(env) if else_block
      end
    end
  end

  class WhileStmnt < ASTList
    alias :condition :first
    alias :body :second

    def to_s
      "(while #{condition} #{body})"
    end

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
    alias :operand :first

    def to_s
      "- #{operand}"
    end

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
    alias :parameters :second
    alias :body :third
    def_delegator :first, :token, :name

    def to_s
      "(def #{name} #{parameters} #{body})"
    end
  end

  class ParameterList < ASTList
  end

  class PrimaryStmnt < ASTList
    def self.create(*args)
      args.first if args.size == 1
    end
  end

  class Postfix < ASTList
  end

  class Arguments < Postfix
  end
end
