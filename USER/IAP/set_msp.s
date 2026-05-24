    AREA    |.text|, CODE, READONLY
    THUMB
    EXPORT  SetMSP

SetMSP
    MSR     MSP, r0
    BX      lr
    ALIGN
    END
