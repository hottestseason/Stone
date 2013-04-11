module Stone
  class Token
    attr_reader :line_number, :value

    def initialize(line_number, value)
      @line_number = line_number
      @value = value
    end
  end

  class NumberToken < Token
    def initialize(line_number, value)
      super
      @value = value.to_i
    end
  end

  class IdToken < Token
  end

  class StringToken < Token
  end
end
