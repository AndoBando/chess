##PGN to FEN converter##

To run, just compile it with any C compiler. I don't think you should have any problems.

Usage: `./combo (input files...) [-ehkops0] [file...]`


Works on multiple games in the same or separate files.
By default, reads from stdin and outputs to stdout.
Pass as many input files as you like

`-h`, help (*this)

`-0`, no arguments intended. Without this argument, if none are given the program throws an error

`-p`, pretty output, outputs colored ascii board in place of fen

`-o file`, outputs to file instead of stdout

`-e file`, redirects errors to file, instead of stderr

`-s string`, rather than from standard starting positions starts each game from the board given by the fen string

`-k`, keep tags, keeps the tags given in the pgn file, by default they are thrown out
