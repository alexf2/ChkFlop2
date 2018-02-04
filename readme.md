## Algo Puzzles
The solution represents an ASP.Net MVC Core 2 platform for demonstrating algorithms. The main distinguishing and exciting feature is that the platform has the metadata-driven UI. I.e. you do not have to use much attributes on algorithms' input and output structures or to implement views to edit input arguments and to display a result. They are automatically derived off the minimal metadata and off the data structures, which are compliant to certain requirements. Therefore, you may easily plug in new algorithms without much care about the UI. It makes it friction-less, as you only need to add your algorithm code and implement a compliant descriptor class, which provides minimal metadata and describes what input parameters algorithm needs to receive and what results it produces.
The idea suits well for comparing the flexibility of different languages and platforms to produce generic UI with minimal hardcoding. This instance implemented in .Net/C#/Asp.Net Core 2, which is a typed language for which working with arbitrary data is a kind of not a friction-less task. However, it turned out to be feasible to implement a metadata-driven UI without much extra coding and without overriding embedded standard Razor templates. The task solved by adding some type convertors, providers and templates for the types, needed in algorithms. Now it supports: 
* array of int, string;
* jagged array of int[], string[];
* list of int, string;
* list of int[], string[];
* single linked list of int;
* matrix of int, string.

Types are plugged in via a convertor in 
[Startup.cs: RegisterTypeConverters](./AlgoPuzzles/Startup.cs).
```csharp
CM.TypeDescriptor.AddAttributes(typeof(int[]), new CM.TypeConverterAttribute(typeof(SemicolonSeparatedArrayConvertor<int>)));
```

Also, Razor templates to display arrays, matrixes and lists of arrays are in [Views/Shared/DisplayTemplates](./AlgoPuzzles/Views/Shared/DisplayTemplates/). They are matched to the types by means of [UiHintMetadataProvider](./AlgoPuzzles/Helpers/UiHintMetadataProvider.cs), which is plugged in at Startup.cs:
```csharp
services.AddMvc()
	.AddMvcOptions(m => m.ModelMetadataDetailsProviders.Add(new UiHintMetadataProvider()))
```

UiHintMetadataProvider facilitates automatic selecting a right Razor template for each complex type and this approach works against default Object Rzaor template, so you do need to decorate properties in your POCOs by UIHintAttribute.

### How to add an algorithm
You need to follow the steps.
1. Wrap algorithm into your class inheriting AlgoBase<YourClass.Args>.
1. Put it into [Algorithms/Implementation/ folder](./Algorithms/Implementation/).
1. Inside of YourClass define a class for the input arguments as a POCO. In examples it is usually called Args.
1. Override Name and Description properties to provide readable information, describing your algorithm.
1. Override ExecuteCore method where you call your algorithm and return a result as an anonymous type.
1. Override TestsSet property to return a set of test cases.
1. Overriding FileName is needed to return your source code to display in UI, but do not forget to mark your CS file as "copy always".
 
```csharp
    public class FindCommonNumbersInArrays : AlgoBase<FindCommonNumbersInArrays.Args>
    {
        static readonly int[] EmptyArray = new int[] { };

        public sealed class Args
        {
            [Display(Name = "First Array")]
            public int[] A1 { get; set; } = EmptyArray;

            [Display(Name = "Second Array")]                                    
            public int[] A2 { get; set; } = EmptyArray;

            [Display(Name = "Third Array")]            
            public int[] A3 { get; set; } = EmptyArray;
        }        

        public override string Name { get => "Common array numbers"; }

        public override string Description { get => @"Given three positive int arrays of unique numbers. 
 There is a need to find common numbers of all the arrays.
<br/><pre>Example: [12; 11; 15; 8], [7; 3; 11; 18; 8], [1; 2; 11; 7; 8] --> [11, 8]</pre>"; }

        protected override dynamic ExecuteCore(Args input)
        {            
            return new { CommonNumbers = TestCommonNumbersThree.FindCommonNumbers(input.A1, input.A2, input.A3) };
        }

        public override IEnumerable<Args> TestSet { get => new [] {
                new Args(){A1 = new []{1,7,4}, A2 = new []{18, 7, 22, 1}, A3 = new []{18, 1, 4, 5, 7} },
                new Args(){A1 = new []{1,7,4}, A2 = new int[]{}, A3 = new []{2, 1, 4, 5, 7} },
                new Args(){A1 = new []{11,12,11, 3, 7, 9, 15, 1, 10, 2}, A2 = new []{1, 12, 11, 9, 55, 88, 99}, A3 = new []{9, 1, 88, 121, 12} }
            };
        }        

        public override string FileName { get => base.GetFileName(); }
    }

```