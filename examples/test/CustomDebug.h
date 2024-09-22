#ifndef CustomDebug_h
#define CustomDebug_h

#define debughex(A) Serial.print(A, 16);

#define debugln1(A) Serial.println(A);

#define debugln2(A, B) Serial.print(A);\
                                Serial.print(' ');\
                                Serial.println(B);

#define debugln3(A, B, C) Serial.print(A);\
                                Serial.print(' ');\
                                Serial.print(B);\
                                Serial.print(' ');\
                                Serial.println(C);

#define debugln4(A, B, C, D) Serial.print(A);\
                                Serial.print(' ');\
                                Serial.print(B);\
                                Serial.print(' ');\
                                Serial.print(C);\
                                Serial.print(' ');\
                                Serial.println(D);

#define debugln5(A, B, C, D, E) Serial.print(A);\
                                Serial.print(' ');\
                                Serial.print(B);\
                                Serial.print(' ');\
                                Serial.print(C);\
                                Serial.print(' ');\
                                Serial.print(D);\
                                Serial.print(' ');\
                                Serial.println(E);

#define debugln6(A, B, C, D, E, G) Serial.print(A); Serial.print(' ');\
                                Serial.print(B); Serial.print(' ');\
                                Serial.print(C); Serial.print(' ');\
                                Serial.print(D); Serial.print(' ');\
                                Serial.print(E); Serial.print(' ');\
                                Serial.println(G);

#define debug1(A)    Serial.print(A); Serial.print(' ');

#define debug2(A, B) Serial.print(A); Serial.print(' '); \
                            Serial.print(B); Serial.print(' ');

#define debug3(A, B, C) Serial.print(A); Serial.print(' ');\
                                Serial.print(B); Serial.print(' ');\
                                Serial.print(C); Serial.print(' ');

#define debug4(A, B, C, D) Serial.print(A); Serial.print(' ');\
                                Serial.print(B); Serial.print(' ');\
                                Serial.print(C); Serial.print(' ');\
                                Serial.print(D); Serial.print(' ');

#define debug5(A, B, C, D, E) Serial.print(A); Serial.print(' ');\
                                Serial.print(B); Serial.print(' ');\
                                Serial.print(C); Serial.print(' ');\
                                Serial.print(D); Serial.print(' ');\
                                Serial.print(E); Serial.print(' ');

#define debug6(A, B, C, D, E, G) Serial.print(A); Serial.print(' ');\
                                Serial.print(B); Serial.print(' ');\
                                Serial.print(C); Serial.print(' ');\
                                Serial.print(D); Serial.print(' ');\
                                Serial.print(E); Serial.print(' ');\
                                Serial.print(G); Serial.print(' ');

#endif