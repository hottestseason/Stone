require_relative '../lib/stone.rb'

lexer = Stone::Lexer.new(open(ARGV[0]))
e = Stone::Parser.new(lexer.tokens)
env = {}
while(tree = e.program) do
  puts tree.eval(env)
end
