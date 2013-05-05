module Stone
  class Lexer
    COMMENT_PATTERN = '//.*'
    NUMBER_LITERAL_PATTERN = '[0-9]+'
    ID_LITERAL_PATTEN = '[A-Z_a-z][A-Z_a-z0-9]*|==|<=|>=|&&|\|\||\W'
    STRING_LITERAL_PATTERN = '"([^\"])*"'
    REGEXP = /\s*((#{COMMENT_PATTERN})|(#{NUMBER_LITERAL_PATTERN})|(#{ID_LITERAL_PATTEN})|#{STRING_LITERAL_PATTERN})?/

    attr_reader :tokens

    def initialize(io)
      @tokens = []
      parse io
    end

    private
    def parse(io)
      io.each_line.with_index do |line, i|
        line.scan(REGEXP) do |matched, comment, number_literal, id_literal, string_literal|
          if number_literal
            @tokens << NumberToken.new(i, number_literal)
          end
          if id_literal
            @tokens << IdToken.new(i, id_literal)
          end
          if string_literal
            @tokens << StringToken.new(i, string_literal)
          end
        end
        @tokens << IdToken.new(i, "\n")
      end
    end
  end
end
