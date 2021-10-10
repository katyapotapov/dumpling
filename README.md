# Dumpling

Interactive adventure slideshow program.

## TODO
- Build a simple editor which allows placing images and text
- Add error checking for data loading
- Add documentation for data commands
- Add `automover` which can have animated position based on time
- Allow question to have different pages for different input

## DONE
- Add command to copy a previous page into current page
    - This is done using `bpage {page name} {base page name}`
- Entities with the same name should override previous entities defined on the same page
- Add keyboard mover which allows moving an entity based on keyboard input
    - This is done using `mover {mover name} {entity name} {speed float}`
- Add collision handler to go to a page when an entity touches something
    - This is done using `touch {touchable name} {toucher name} {touched name} {page name}`
