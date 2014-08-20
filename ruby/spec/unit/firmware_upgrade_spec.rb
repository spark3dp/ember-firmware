require 'spec_helper'

module Smith::Config
  describe Firmware do
    context 'when upgrading', :tmp_dir do

      let(:firmware_dir) { File.join(@tmp_dir_path, 'firmware') }
      let(:prior_backup_image) { File.join(firmware_dir, 'smith-0.0.0.img') }
      let(:backup_image) { File.join(firmware_dir, 'smith-0.0.1.img') }

      before do
        FileUtils.mkdir(firmware_dir)
        ENV['FIRMWARE_DIR'] = firmware_dir
        FileUtils.touch(backup_image)
      end

      shared_examples_for 'firmware upgrade' do
        
        context 'when specifed package does not exist' do

          it 'raises appropriate error' do
            expect { subject.upgrade('foo') }.to raise_error(Firmware::UpgradeError)
          end

        end

        context 'when specified package is valid and firmware directory is clean' do

          before { subject.upgrade(resource('smith-0.0.2-valid.tar.gz')) }

          it 'places new image in firmware directory' do
            expect(File.file?(File.join(firmware_dir, 'smith-0.0.2.img'))).to eq(true)
          end

          it 'updates versions file, with upgraded image as the primary entry and the previous primary entry as the backup entry ' do
            version_entries = File.readlines(File.join(firmware_dir, 'versions'))

            expect(version_entries[0]).to eq("3fe0542abe1724b68788bbb73e95db39  smith-0.0.1.img\n") # Previous primary entry, now the backup
            expect(version_entries[1]).to eq("3749f52bb326ae96782b42dc0a97b4c1  smith-0.0.2.img\n") # Upgraded firmware entry, now the primary
          end

          it 'removes the prior backup image' do
            expect(File.file?(prior_backup_image)).to eq(false) 
          end

          it 'removes the temporary directory created during package extraction' do
            expect(File.exists?(File.join(firmware_dir, 'smith-0.0.2-valid'))).to eq(false)
          end
          
          it 'does not remove the prior primary image' do
            expect(File.file?(backup_image)).to eq(true) 
          end

        end

        context 'when specified package is valid and extraction directory exists' do
          it 'removes old extraction directory' do
            FileUtils.mkdir(File.join(firmware_dir, 'smith-0.0.2-valid'))
            subject.upgrade(resource('smith-0.0.2-valid.tar.gz'))
          end
        end

        context 'when md5 checksum of firmware image does not match provided checksum' do

          it 'raises appropriate error and removes temporary directory created during package extraction' do
            expect { subject.upgrade(resource('smith-0.0.2-invalid_checksum.tar.gz')) }.to raise_error(Firmware::UpgradeError)
            expect(File.exists?(File.join(firmware_dir, 'smith-0.0.2-invalid_checksum'))).to eq(false)
          end

        end

        context 'when firmware image corresponding to provided version does not exist' do

          it 'raises appropriate error and removes temporary directory created during package extraction' do
            expect { subject.upgrade(resource('smith-0.0.2-missing_image.tar.gz')) }.to raise_error(Firmware::UpgradeError)
            expect(File.exists?(File.join(firmware_dir, 'smith-0.0.2-missing_image'))).to eq(false)
          end

        end 

        context 'when md5sum file does not exist' do

          it 'raises appropriate error and removes temporary directory created during package extraction' do
            expect { subject.upgrade(resource('smith-0.0.2-missing_md5sum.tar.gz')) }.to raise_error(Firmware::UpgradeError)
            expect(File.exists?(File.join(firmware_dir, 'smith-0.0.2-missing_md5sum'))).to eq(false)
          end
        
        end

        context 'when md5sum file does not contain expected contents' do
          
          it 'raises appropriate error and removes temporary directory created during package extraction' do
            expect { subject.upgrade(resource('smith-0.0.2-malformed_md5sum.tar.gz')) }.to raise_error(Firmware::UpgradeError)
            expect(File.exists?(File.join(firmware_dir, 'smith-0.0.2-malformed_md5sum'))).to eq(false)
          end

        end

        context 'when specified package is not a valid gzipped tarball' do

          it 'raises appropriate error and removes temporary directory created during package extraction' do
            expect { subject.upgrade(resource('smith-0.0.2-invalid_archive.tar.gz')) }.to raise_error(Firmware::UpgradeError)
          end

        end

      end

      context 'when versions file has two entries' do
        before do
          FileUtils.copy(resource('versions-two_entries'), Smith::Config.firmware_versions_file)
          FileUtils.touch(prior_backup_image)
        end

        it_behaves_like 'firmware upgrade' 
      end



      context 'when versions file has one entry' do
        before do
          FileUtils.copy(resource('versions-one_entry'), Smith::Config.firmware_versions_file)
        end
        
        it_behaves_like 'firmware upgrade' 
      end

    end
  end
end
