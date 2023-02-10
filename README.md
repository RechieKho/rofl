# Richie's Obvious Figure Language

Richie's Obvious Figure Language, or ROFL is another simple language for
configurations in C. TOML, YAML, INI and JSON did a great job but I want 
a language that serialize and deserialize complex data structure with 
only one source (`.c`) file and one header (`.h`) file and *NO* memory
allocation.

### Specification
#### Examples
Here is an example of data written in ROFL.

```
# Object
number = 1
string = "Hello world"
array = 1, 2, 3
array_to_convert_to_color = Color 128, 128, 128

    # Nested Object
    position = Vector3 200, 200, 200
```

If roughly translated to JSON, it would be like this:

```json
{
    "Object" : {
        "number": 1, 
        "string": "Hello world",
        "array": [1, 2, 3]
        "array_to_convert_to_color": [128, 128, 128],
        "Nested Object": {
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
type. In the example, we see there are some arrays prefixed with some text, in
this case, `Vector3`.

```
position = Vector3 200, 200, 200
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

`#` starts an object. Indentation means nesting. Don't mix tab and spaces.
