# YetAnotherShell
##Lexer
-Infinite state machine
-Initial, Running, Succeed, Failed
-Tokens
  -PARENTL (
  -PARENTR )
  -LT <
  -RT >
  -PIPE |
  -BACKGROUND &
  -\n \t ' ' /*blank*/
 ##Parser
 -Recursive descent
 -Grammer
  -Simple cmd
    -;word-list
    -;
  -Redir cmd
    -: simple-cmd RT WORD LT WORD
    -| simple-cmd LT WORD RT WORD
    -| simple-cmd LT WORD
    -| simple-cmd RT WORD
    -| simple-cmd
    -;
  -Pipe cmd
    -: redir-cmd
    -| redir-cmd PIPE pipe-cmd
