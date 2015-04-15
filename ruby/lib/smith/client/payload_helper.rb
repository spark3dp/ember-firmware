# Mix in to provide helper methods for building payloads
# for command acknowledgement and status reports

module Smith
  module Client
    module PayloadHelper

      def status_payload(status)
        data = status.clone
        data.delete(SPARK_STATE_PS_KEY)
        data.delete(SPARK_JOB_STATE_PS_KEY)
        data.delete(LOCAL_JOB_UUID_PS_KEY)
        {
            printer_status: status[SPARK_STATE_PS_KEY],
            error_code: status[ERROR_CODE_PS_KEY],
            error_message: status[ERROR_MSG_PS_KEY],
            data: data
        }.merge!(job_specific_payload(status))
      end

      def command_payload(command, command_state, message, status, job_id = nil)
        {
            printer_status: status[SPARK_STATE_PS_KEY],
            progress:   if command_state == Command::RECEIVED_ACK
                          0
                        else
                          1 # command completed or failed
                        end,
            error_code: if command_state == Command::FAILED_ACK
                          500
                        else
                          0 # command received or completed without error
                        end,
            error_message: message,
            data: {
                command: command,
                message: message,
                state: command_state
            }
        }.merge!(job_specific_payload(status, job_id, command_state))
      end

      def job_specific_payload(status, job_id_from_command = nil, command_state = nil)
        if job_id_from_command
          # job_id_from command indicates this was a print data command with the given job ID
          job_status = if command_state == Command::FAILED_ACK
                         'failed'
                       else
                         'received' # even though the actual print data may not have been received yet
                       end
          return { job_id: job_id_from_command, job_status: job_status, job_progress: 0.0 }
        elsif !status[SPARK_JOB_STATE_PS_KEY].empty?
          # non-empty spark job state indicates there is a job in progress,
          # or at least printable data available for a "local" job
          job_id = if status[JOB_ID_PS_KEY].empty?
                     'local_' + status[LOCAL_JOB_UUID_PS_KEY]
                   else
                     status[JOB_ID_PS_KEY]
                   end
          job_progress = if status[TOTAL_LAYERS_PS_KEY] == 0
                           0.0
                         else
                           status[LAYER_PS_KEY].to_f / status[TOTAL_LAYERS_PS_KEY].to_f
                         end
          return { job_id: job_id, job_status: status[SPARK_JOB_STATE_PS_KEY], job_progress: job_progress }
        else
          # no job in progress
          return { }
        end
      end

    end
  end
end
