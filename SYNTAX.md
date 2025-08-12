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

### If Statements

`if (condition) { STATEMENTS }`
`if (condition) { STATEMENTS } else { STATEMENTS }`
`if (condition) { STATEMENTS } else if (condition) { STATEMENTS } ... else { STATMENTS }`

### Switch Statement

`switch (expression) { case (expression) { STATEMENTS } ... default { STATEMENTS } }`

### For Statements

`for (declaration; condition; iteration) { STATEMENTS }`
`for (declaration : iterator) { STATEMENTS }`

### While Statement

`while (condition) { STATEMENTS }`

### Return Statement

`return expression;`

### Break Statement

`break;`

### Continue Statement

`continue;`
