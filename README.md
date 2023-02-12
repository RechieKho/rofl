# Richie's Obviously Fantastic Language
Richie's Obviously Fantastic Language, or ROFL is another simple language for
configurations in C. TOML, YAML, INI and JSON did a great job but I want 
a a dead simple configuration language that parses with one small source (`.c`) file 
and one header (`.h`) file and 0 memory allocation.

### Features
- *No* memory allocation.
- One small (420 lines including comments)  source file and one header file.

### Using ROFL parser
To use it, just copy `rofl.c` and `rofl.h` into your project. 
Write a callback function that handles the members. 
Do note that the callback is called for each members.
Call `parse_rofl` on the text you want to parse.
Error logging can be turn on or off when calling.
Handle error if `parse_rofl` returns error.

### Specification
#### Examples
Here is an example of data written in ROFL.

```
# Object
number = 1
string = "Hello world"
array = 1 2 3
array_to_convert_to_color : Color = 128 128 128

    # NestedObject
    position : Vector3 = 200 200 200
```

If roughly translated to JSON, it would be like this:

```json
{
    "Object" : {
        "number": 1, 
        "string": "Hello world",
        "array": [1, 2, 3]
        "array_to_convert_to_color": [128, 128, 128],
        "NestedObject": {
            "position": [200, 200, 200]
        }
    }
}
```

#### Data types
There are only 2 primitive data types, strings and numbers. In C's point of view,
Strings are pointers point to the original text (no malloc!), while numbers are
`double`. 

For arrays, they are same as C's array, in which the elements must have the same
type.

Moreover, you can add type hint to the member using colon `:`.
```
position : Vector3 = 200, 200, 200
```

`Vector3` is a hint given to the C programmers to decide how to interpret
the array. So here it is telling the C programmers to preceive it as a
`Vector3`, which should have `x`, `y` and `z` component. With the 
interpretation, instead of a normal array, it is like this:

```json
{
    "position": {
        "x": 200,
        "y": 200,
        "z": 200
    }
}
```

Name of objects and key of members *should be in one word*.

`#` starts an object. Indentation means nesting. Don't mix tab and spaces.
