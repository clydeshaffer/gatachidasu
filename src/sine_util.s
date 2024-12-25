.import _sineRadius8
.import _sineRadius11
.import _sineRadius16
.import _sineRadius18
.import _sineRadius23

.export _getSine
.export _setSineMode

.PC02

.segment "RODATA"

.define SineTables _sineRadius8, _sineRadius11, _sineRadius16, _sineRadius18, _sineRadius23

sineTablesLo: .lobytes SineTables
sineTablesHi: .hibytes SineTables

.segment "DATA"

.proc _getSine: near
    SEI
    AND #$7F
    TAX
sineFetch:
    LDA _sineRadius8, x
    CLI
    RTS

.endproc

.segment "CODE"

.proc _setSineMode: near
    SEI
    TAX
    LDA sineTablesLo, x
    STA _getSine::sineFetch+1
    LDA sineTablesHi, x
    STA _getSine::sineFetch+2
    CLI
    RTS

.endproc