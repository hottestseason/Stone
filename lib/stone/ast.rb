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
  end

  class Name < ASTLeaf
  end

  class StringLiteral < ASTLeaf
  end

  class ASTList
    attr_reader :children

    def initialize(*args)
      @children = args
    end

    def to_s
      children.to_s
    end
  end

  class BinaryExpr < ASTList
    def to_s
      "(#{left} #{operator} #{right})"
    end

    def left; children.first end
    def operator; children.second end
    def right; children.third end
  end

  class IfStmnt < ASTList
    def to_s
      "(if #{condition} #{then_block} else #{else_block})"
    end

    def condition; children.first end
    def then_block; children.second end
    def else_block; children.third end
  end

  class WhileStmnt < ASTList
    def to_s
      "(while #{condition} #{body})"
    end

    def condition; children.first end
    def body; children.second end
  end

  class BlockStmnt < ASTList
    def to_s
      children.map { |child| "(#{child})" }.join(" ")
    end
  end

  class NegativeExpr < ASTList
  end

  class NullStmnt < ASTList
    def to_s
      ""
    end
  end
end
