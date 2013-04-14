module Stone
  class Token
    attr_reader :line_number, :value

    def initialize(line_number, value)
      @line_number = line_number
      @value = value
    end

    def to_s
      "#{type(abbr: true)}(#{value})"
    end

    def type(abbr: false)
      nil
    end

    # support #is_(type)? and #must_be_a_(type)!
    def method_missing(method_name, *args)
      case method_name
      when /^is_(?<type>.+)\?$/
        type.to_s == $~[:type] && (args.blank? || args.include?(value))
      when /^must_be_(?<type>.+)!/
        if send("is_#{$~[:type]}?", *args)
          self
        else
          raise
        end
      else
        super
      end
    end
  end

  class NumberToken < Token
    def initialize(line_number, value)
      super
      @value = value.to_i
    end

    def type(abbr: false)
      :number
    end
  end

  class IdToken < Token
    def type(abbr: false)
      abbr ? :id : :identifier
    end
  end

  class StringToken < Token
    def type(abbr: false)
      :string
    end
  end
end
