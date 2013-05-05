module Stone
  class Function
    attr_reader :parameters, :body

    def initialize(parameters, body, outer_env)
      @parameters = parameters
      @body = body
      @outer_env = outer_env
    end

    def create_local_env
      Environment.new(@outer_env)
    end
  end
end
