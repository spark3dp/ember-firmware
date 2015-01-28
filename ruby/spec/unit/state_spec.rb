require 'common_helper'
require 'smith/state'

module Smith
  describe State, :tmp_dir do

    let(:state_file) { tmp_dir("#{Time.now.to_i}#{rand(1000)}settings") }

    before do
      # Ensure the backing file is empty
      File.write(state_file, {}.to_json)
      Smith::Settings.state_file = state_file
    end

    it 'allows access to fields' do
      subject.printer_id = 123
      expect(subject.printer_id).to eq(123)
    end

    context 'when saved and loaded' do
      it 'updates fields with persisted values' do
        subject.printer_id = 123
        subject.save
        expect(described_class.load.printer_id).to eq(123)
      end
    end

    context 'when loaded when backing file does not exist' do
      it 'returns empty state object' do
        File.delete(state_file)
        expect(described_class.load.printer_id).to be_nil
      end
    end

    context 'when updated with multiple values' do
      it 'persists state after setting fields to specified values' do
        subject.update(a: 1, b: '2')
        state = described_class.load
        expect(state.a).to eq(1)
        expect(state.b).to eq('2')
      end
    end

  end
end
