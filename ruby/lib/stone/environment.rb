module Stone
  class Environment
    def initialize(outer = nil)
      @values = {}
      @outer = outer
    end

    def get(name)
      if !@values.key?(name) && @outer
        @outer.get(name)
      else
        @values[name]
      end
    end

    def put(name, value)
      (where?(name) || self).create(name, value)
    end

    def create(name, value)
      @values[name] = value
    end

    def where?(name)
      if @values.key?(name)
        self
      else
        @outer.try?(:where?, name)
      end
    end
  end
end
