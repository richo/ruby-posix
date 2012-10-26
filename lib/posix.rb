require 'posix/posix'

Dir["#{File.expand_path("../posix/", __FILE__)}/*"].each do |file|
  require file
end
