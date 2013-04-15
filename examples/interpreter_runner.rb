require_relative '../lib/stone.rb'

lexer = Stone::Lexer.new(open(ARGV[0]))
e = Stone::Parser.new(lexer.tokens)
env = Stone::Environment.new
while(tree = e.program) do
  puts tree.eval(env)
end
