require 'posix'

mask = Posix::Sigset.new
mask << "INT"
mask << "USR2"

puts Posix.sigprocmask(Posix::SIG_SETMASK, mask)

Posix.execve("/bin/bash", ["/bin/bash"], {"rawr" => "Thing"})
