# Mix in to provide helper methods for building payloads
# for command acknowledgement and status reports

module Smith
  module Client
    module PayloadHelper

      def status_payload(status)
        payload = {
            printer_status: status[SPARK_STATE],
            error_code: status[ERROR_CODE_PS_KEY],
            error_message: status[ERROR_MSG_PS_KEY],
            data: status
        }
        if !status[SPARK_JOB_STATE].empty?
          job_id = if status[JOB_ID_PS_KEY].empty?
            'local'
          else
            status[JOB_ID_PS_KEY]
          end
          job_progress = if status[TOAL_LAYERS_PS_KEY] == 0
            0.0
          else
            status[LAYER_PS_KEY].to_f / status[TOAL_LAYERS_PS_KEY].to_f
          end
          payload.merge!(job_id: job_id, job_status: status[SPARK_JOB_STATE], job_progress: job_progress)
        end
        return payload
      end

      def command_payload(status)

      end

    end
  end
end
