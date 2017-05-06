# Virtual Machine

This is a C implementation of my [original StackVM](https://github.com/KCreate/stackvm)
which is written in Crystal. It serves as a way of learning C, while learning more and more
about virtual machines in general.

## Installation

```bash
git clone https://github.com/KCreate/c-stackvm
make vm

bin/vm
```

## Usage

Keep in mind, this repository only contains the virtual machine. To generate an executable
for it, you will need the assembler from my
[original repository](https://github,com/KCreate/stackvm) written in Crystal.

```bash
bin/vm myprogram.bc
```

## Contributing

1. Fork it ( https://github.com/KCreate/c-stackvm/fork )
2. Create your feature branch (git checkout -b my-new-feature)
3. Commit your changes (git commit -am 'Add some feature')
4. Push to the branch (git push origin my-new-feature)
5. Create a new Pull Request

## Contributors

- [KCreate](https://github.com/KCreate) Leonard Schuetz - creator, maintainer
