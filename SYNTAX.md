All statements must end in `;`

### Type

`name`
`name<type, ...>`
`name[]`

### Literal

`integer`
`float`
`string`

### Code Block

`(type <expansion>name <default>, ...) { STATEMENTS }`
`{ STATEMENTS }`

### Operations

Precendence of Operations

1. `^`

2. `!, -, +` (unary)

3. `*, /, $, %`

4. `+, -`

5. `==, !=, <=, >=, <, >`

6. `&`

7. `|`

8. `=, +=, -=, *=, /=`

### Get Variable

`name`

### Assign Variable

`qualifiers type name;`
`qualifiers type name = EXPRESSION;`

### Function Call

`name(EXPRESSION, ...);`

### Define Function

`qualifiers return_type name(type <expansion>name <default>, ...);`
`qualifiers return_type name(type <expansion>name <default>, ...) { STATEMENTS }`
