#  File: firmware_upgrade_spec.rb
#
#  This file is part of the Ember Ruby Gem.
#
#  Copyright 2015 Autodesk, Inc. <http://ember.autodesk.com/>
#
#  Authors:
#  Jason Lefley
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  THIS PROGRAM IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL,
#  BUT WITHOUT ANY WARRANTY; WITHOUT EVEN THE IMPLIED WARRANTY OF
#  MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  SEE THE
#  GNU GENERAL PUBLIC LICENSE FOR MORE DETAILS.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.

require 'config_helper'

module Smith::Config
  describe Firmware do
    context 'when upgrading', :tmp_dir do

      include FirmwareUpgradeHelper

      subject { described_class }
      let(:prior_backup_image) { File.join(firmware_dir, 'smith-0.0.0.img') }
      let(:backup_image) { File.join(firmware_dir, 'smith-0.0.1.img') }
      let(:new_image) { File.join(firmware_dir, 'smith-0.0.2.img') }
      let(:firmware_dir_contents) { Dir[File.join(firmware_dir, '**/*')] }

      before { FileUtils.touch(backup_image) }

      shared_examples_for 'firmware upgrade' do
        
        context 'when specified package is valid and firmware directory is clean' do

          before { subject.upgrade(resource('smith-0.0.2-valid.tar')) }

          it 'places new image in firmware directory' do
            expect(File.file?(new_image)).to eq(true)
          end

          it 'updates versions file, with upgraded image as the primary entry and the previous primary entry as the backup entry ' do
            version_entries = File.readlines(firmware_versions_file)

            expect(version_entries[0]).to eq("3fe0542abe1724b68788bbb73e95db39  smith-0.0.1.img\n") # Previous primary entry, now the backup
            expect(version_entries[1]).to eq("3749f52bb326ae96782b42dc0a97b4c1  smith-0.0.2.img\n") # Upgraded firmware entry, now the primary
          end

          it 'removes old files and temp files from firmware directory' do
            expect(firmware_dir_contents).to match([backup_image, new_image, firmware_versions_file])
          end

        end

        context 'when md5 checksum of firmware image does not match provided checksum' do

          it 'raises appropriate error' do
            expect { subject.upgrade(resource('smith-0.0.2-invalid_checksum.tar')) }.to raise_error(Firmware::UpgradeError)
          end

        end

        context 'when firmware image corresponding to provided version does not exist' do

          it 'raises appropriate error' do
            expect { subject.upgrade(resource('smith-0.0.2-missing_image.tar')) }.to raise_error(Firmware::UpgradeError)
          end

        end 

        context 'when upgrade package does not contain md5sum file' do

          it 'raises appropriate error' do
            expect { subject.upgrade(resource('smith-0.0.2-missing_md5sum.tar')) }.to raise_error(Firmware::UpgradeError)
          end
        
        end

        context 'when md5sum file does not contain expected contents' do
          
          it 'raises appropriate error' do
            expect { subject.upgrade(resource('smith-0.0.2-malformed_md5sum.tar')) }.to raise_error(Firmware::UpgradeError)
          end

        end

        context 'when upgrade package contains a firmware image having the same version as the current firmware version' do
          it 'raises appropriate error and does not overwrite current firmware image file' do
            expect { subject.upgrade(resource('smith-0.0.1.tar')) }.to raise_error(Firmware::UpgradeError)
            expect(File.read(backup_image)).to eq('')
          end
        end

      end

      context 'when versions file has two entries' do
        before do
          set_two_entry_versions_file
          FileUtils.touch(prior_backup_image)
        end

        it_behaves_like 'firmware upgrade' 
      end



      context 'when versions file has one entry' do
        before do
          set_one_entry_versions_file
        end
        
        it_behaves_like 'firmware upgrade' 
      end

    end
  end
end
