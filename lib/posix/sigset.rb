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
    end

    def include?(signal)
      @signals.include? signal
    end

  end
end
