from d7a.alp.command import SetFileProperties
from d7a.d7anp.addressee import IdType
from d7a.alp.operations.requests import WriteFileData, ReadFileData
from d7a.d7anp.session import Session
from d7a.util.iframe import Iframe
from d7a.alp.control import Control

new_file_id = 0x02 #set file ID here
set_file_properties_command = SetFileProperties(file_id=new_file_id)

# Create an ALP command
alp_command = set_file_properties_command

# Set control parameters for transmission
control = Control()
control.target_address = IdType.NOID
control.interface_status = Control.InterfaceStatus.NORMAL
control.dialog_start = True
control.dialog_template = False
control.response_expected = True
control.dialog_id = 0x00  # Set a unique dialog ID

# Encapsulate the ALP command and control parameters in an Iframe
ifrm = Iframe(ctrl=control, alp=alp_command)

response = Session.request(ifrm)

read_file_command = ReadFileData(file_id=new_file_id, offset=0, length=1)  # Adjust the offset and length as needed
response = Session.request(read_file_command)