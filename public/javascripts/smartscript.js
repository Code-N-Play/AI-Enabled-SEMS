document.addEventListener('DOMContentLoaded', function(){
    enableSocketStatusChange(".statusChangeSwitch");
});

function enableSocketStatusChange(selector){

    const switches = document.querySelectorAll(selector);

    switches.forEach(s => {
        const card = s.closest(".socket-card");
        const statusBtn = card.querySelector('.status');

        s.addEventListener('change', function(){
            const slug = s.getAttribute("data-slug");

            console.log(s.checked ? 'active' : 'inactive', slug);

            fetch(`/sockets/${slug}/status`, {
                method: 'PATCH',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify({ isActive:  s.checked })
            })
            .then(response => response.json())
            .then(data => {
                console.log(data);
                if(data.success){
                    statusBtn.textContent = data.data.isActive ? "● Online" : "● Offline";
                    statusBtn.classList.remove("online");
                     statusBtn.classList.remove("offline");
                    statusBtn.classList.add(data.data.isActive ? 'online' : 'offline');
                }else{

                }
            })
            .catch(error => {
                
                console.error('Error -----:', error);
            });

        });
    });
}
