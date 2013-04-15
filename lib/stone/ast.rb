module Stone
  class ASTree
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

    delegate :each, :[], :size, :second, :third, to: :children

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

    def operator
      second.token.value
    end

    def to_s
      "(#{left} #{operator} #{right})"
    end

    def eval(env)
      if operator == "="
        name = left.token.value
        env.create(name, right.eval(env))
      else
        Kernel.eval "#{left.eval(env)} #{operator} #{right.eval(env)}"
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

    def name
      first.token.value
    end

    def to_s
      "(def #{name} #{parameters} #{body})"
    end

    def eval(env)
      env.create(name, Function.new(parameters, body, env))
    end
  end

  class ParameterList < ASTList
    def to_s
      children.map { |child| "(#{child.token.value})" }.join(" ")
    end

    def name(i)
      children[i].token.value
    end

    def eval(env, i, value)
      env.create(name(i), value)
    end
  end

  class PrimaryStmnt < ASTList
    alias :operand :first

    def self.create(*args)
      if args.size == 1
        args.first
      else
        super
      end
    end

    def eval(env)
      eval_sub_expr(env, 0)
    end

    def postfix(nest)
      self[size - nest - 1]
    end

    def eval_sub_expr(env, nest)
      if size - nest - 1 > 0
        postfix(nest).eval(env, eval_sub_expr(env, nest + 1))
      else
        operand.eval(env)
      end
    end
  end

  class Arguments < ASTList
    def eval(env, function)
      local_env = function.create_local_env
      each.with_index do |arg, i|
        function.parameters.eval(local_env, i, arg.eval(env))
      end
      function.body.eval(local_env)
    end
  end
end
