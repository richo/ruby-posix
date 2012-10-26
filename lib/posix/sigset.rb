class Posix
  class Sigset

    def initialize
      @signals = []
    end

    def <<(signal)
      case signal
      when String
        signal = Signal.list[signal]
      end

      @signals << signal unless include? signal
      self
    end

    def include?(signal)
      @signals.include? signal
    end

  end
end
