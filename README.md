# ALL-training
Training of Adaptive LL(*) Parsing


Trial of algorithm described in paper "Adaptive LL(*) Parsing: The Power of Dynamic Analysis"
by Terence Parr,Sam Harwell and Kathleen Fisher.
I extend simple rules definition by repetition options:
 "?" - 0 or 1
 "+" - 1 or more
 "*" - 2 or more
gram09.dat:
S->Ac
S->Ad
A->a+
