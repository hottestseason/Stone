require_relative '../lib/stone.rb'

lexer = Stone::Lexer.new(ARGV[0])
lexer.each_token do |token|
  p token
end
