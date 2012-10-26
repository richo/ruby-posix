Fills some of the gaps in the stdlibrary

A small example of this is:

```ruby
require 'posix'

mask = Posix::Sigset.new
mask << "INT"
mask << "USR2"

puts Posix.sigprocmask(Posix::SIG_SETMASK, mask)

Posix.execve("/bin/bash", ["/bin/bash"], {"rawr" => "Thing"})
```

The notable differences are:
* The environment has becomes the child environmemt, instead of being merged into the current environment awkwardly.
* `sigprocmask` is a thing you can actually call.
* `Posix.execve` doesn't reset sigprocmask before exec'ing, unless `Process#.spawn`, `Kernel#exec` and friends.
