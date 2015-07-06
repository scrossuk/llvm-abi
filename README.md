# llvm-abi

This is a library for generating LLVM IR that complies with platform ABIs.

Typically this is useful for frontends generating code that needs to comply with
a target's language-specific ABI, such as a C ABI. There's a generic interface
for an 'ABI' and implementations for each target (e.g. x86, ARM etc.).

## Why?

LLVM IR doesn't currently have any way for representing language type
information, however platform ABIs are typically expressed in terms of source
language types.

This means backends don't have all the available information for generating
ABI-correct calls/returns so it's up to frontends to emit code that provides the
necessary information to the backend.

Unfortunately this task is quite involved and target-specific, hence this
library has been created to handle this and abstract away the details.

## Who's developing this?

This is currently being developed by [Stephen Cross](http://scross.co.uk) and
was created due to the need for good ABI support in the
[Loci](http://loci-lang.org) [compiler](https://github.com/scross99/locic);
ultimately the aim is that this library is useful to all LLVM frontends that
are interested in ABI compliance.

It is worth noting that many significant pieces of functionality have been
pulled out of [Clang](http://clang.llvm.org/).

If you'd like to contribute, that would be awesome! Also any suggestions or
queries are very welcome (raise a GitHub issue).

Furthermore, it'd be great to add support for languages beyond C.

## Current status

Currently only the 'x86_64' ABI (or AMD64 ABI) is supported, but it should be
relatively straightforward to add support for the other x86 targets. Clearly the
aim is to also add support for the other LLVM targets by extracting the
necessary parts from Clang.

In terms of features, there are at least the following gaping holes in the
library:

* **CPU selection** - There's a lot of functionality in Clang for selecting a
                      generic CPU (we typically don't select the native CPU
                      because similar CPUs with the same architecture may not
                      have the same processor features) and this needs to be
                      brought into the library.
* **Encoding user-specified alignment for types**
* **Receiving varargs parameters**
* **Marking arguments as already in-memory** - There are excessive loads/stores
                                               being generated due to not
                                               recognising the arguments are
                                               already in stack memory.
* **inalloca support** - There are some aspects of functionality in various
                         places but it's very incomplete.

## Testing

The approach to testing is currently to create LLVM IR files that specify an
ABI function type (e.g. `int (double, int)`), which generates some code that's
then checked against the file.

Two functions will be generated: a caller and a callee, both with the
ABI-encoded function signature. The caller function then receives arguments,
decodes them, then immediately re-encodes them and passes them to the callee.
Further it then decodes the return value and then re-encodes it to return.

This testing strategy makes it fairly simple to check that the ABI
implementation is encoding and decoding arguments as expected.

If a test fails then Clang is run to see what it outputs. Essentially the aim is
to always produce output that (functionally) matches Clang.
