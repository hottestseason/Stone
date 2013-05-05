require_relative '../lib/stone.rb'

lexer = Stone::Lexer.new(open(ARGV[0]))
puts lexer.tokens
