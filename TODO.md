### Bugs

-   [ ] Fix bugs in error message code display
-   [ ] Fix segmentation fault issues (caused by tokens being empty)

### Small Changes

-   [ ] Add flags (public, const etc) to variable assigment and function definition (they should have a set order)
-   [ ] Make semicolons compulsory
-   [ ] Allow user to end assignment or function definitions with no body with a semicolon
-   [ ] Create tests for syntax_analyser for when AnalyseSyntax produces no AST (invalid token etc) including recursively (can only be done when all structures implemented)

### Features

-   [ ] Allow multi-assignments in variable assigning
-   [ ] Implement Templates

### Future Reminders

-   Will need test to check for error on line 179 syntax_analyser.cpp when other uses of () implemented
