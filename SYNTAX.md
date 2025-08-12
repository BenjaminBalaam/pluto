All statements must end in ;

### Literal

integer
float
string

### Type

name
name<type, ...>
name[]

### Code Block

(type <expansion>name <default>, ...) { STATEMENTS }
{ STATEMENTS }

### Assign Variable

qualifiers type name;
qualifiers type name = EXPRESSION;

### Get Variable

### Define Function

qualifiers return_type name(type <expansion>name <default>, ...);
qualifiers return_type name(type <expansion>name <default>, ...) { STATEMENTS }

### Function Call

name(EXPRESSION, ...);
