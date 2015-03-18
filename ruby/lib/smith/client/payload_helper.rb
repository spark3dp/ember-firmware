# Mix in to provide helper methods for building payloads
# for command acknowledgement and status reports

module Smith
  module Client
    module PayloadHelper

      def status_payload(status)
        data = status.clone
        data.delete(SPARK_STATE_PS_KEY)
        data.delete(SPARK_JOB_STATE_PS_KEY)
        {
            printer_status: status[SPARK_STATE_PS_KEY],
            error_code: status[ERROR_CODE_PS_KEY],
            error_message: status[ERROR_MSG_PS_KEY],
            data: data
        }.merge!(job_specific_payload(status))
      end

      def command_payload(command, command_state, message, status)
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
        }.merge!(job_specific_payload(status))
      end

      def job_specific_payload(status)
        if !status[SPARK_JOB_STATE_PS_KEY].empty?
          # non-empty spark job state indicates there is a job in progress,
          # or at least printable data available for a "local" job
          job_id = if status[JOB_ID_PS_KEY].empty?
                     'local'
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
