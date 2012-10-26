# vim: ft=ruby

Gem::Specification.new do |s|
  s.name        = "posix"
  s.version     = "0.0.4"
  s.authors     = ["Richo Healey"]
  s.email       = ["richo@psych0tik.net"]
  s.homepage    = "http://github.com/richo/ruby-posix"
  s.summary     = "Wrappers for some posix functions that I couldn't find in the stdlib"
  s.description = s.summary

  #s.add_dependency "some_dependency"

  #s.add_development_dependency "rake"
  #s.add_development_dependency "mocha"
  #s.add_development_dependency "rspec"

  s.files         = `git ls-files`.split("\n")
  s.extensions    = ['ext/posix/extconf.rb']
  s.test_files    = `git ls-files -- {test,spec,features}/*`.split("\n")
  s.require_paths = ["lib"]
end


